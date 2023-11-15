#pragma once

#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <chrono>
#include <thread>
#include <memory>
#include <utility>

#include "graph.hpp"
#include "message_queue.hpp"
#include "random_walker.hpp"
#include "start_flag.hpp"
#include "random_walk_config.hpp"
#include "random_walker_manager.hpp"
#include "cache.hpp"
#include "param.hpp"
#include "util.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class ARWS {

public : 

    // コンストラクタ
    ARWS(const std::string& dir_path);

    // thread を開始させる関数
    void start();

    // RWer 生成 & 処理をする関数
    void generateRWer();

    // RWer の再送制御用スレッドの動作
    void reSendThread();

    // RW を実行する関数
    void executeRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr, StdRandNumGenerator& gen);

    // executeRandomWalk で終了した RWer を処理する関数
    void endRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr);

    // 終了した RWer について, 経路情報からグラフデータにキャッシュを登録する関数
    void checkRWer(std::unique_ptr<RandomWalker>&& RWer_ptr);

    // メッセージ処理用の関数
    void procMessage(const uint16_t& proc_id);

    // send_queue から RWer を取ってきて他サーバへ送信する関数 (送信先毎)
    void sendMessage(const host_id_t& dst_id);

    // 他サーバからメッセージを受信し, message_queue に push する関数 (ポート番号毎)
    void receiveMessage(const uint16_t& port_num);

    // IPv4 サーバソケットを生成 (UDP)
    int createUdpServerSocket(const uint16_t& port_num);

    // IPv4 サーバソケットを生成 (TCP)
    int createTcpServerSocket(const uint16_t& port_num);

    // 実験結果を start_manager に送信する関数
    void sendToStartManager();

    // cache を溜めるための RW 実行
    void genarateRWerForCache();
    

private :

    std::string hostname_; // 自サーバのホスト名
    host_id_t hostip_; // 自サーバの IP アドレス
    host_id_t hostid_;
    std::string hostip_str_; // IP アドレスの文字列
    host_id_t startmanagerip_; // StartManager の IP アドレス
    Graph graph_; // グラフデータ
    std::vector<host_id_t> worker_ip_all_;
    // std::unordered_map<uint16_t, MessageQueue<RandomWalker>> message_queue_; // ポート番号毎の receive キュー
    MessageQueue<char[]>* message_queue_; // ポート番号毎の receive キュー
    // std::unordered_map<uint32_t, MessageQueue<RandomWalker>> send_queue_; // 送信先毎の send キュー
    MessageQueue<RandomWalker>* send_queue_; // 送信先毎の send キュー
    StartFlag start_flag_; // 実験開始の合図に関する情報
    RandomWalkConfig RW_config_; // Random Walk 実行関連の設定
    RandomWalkerManager RW_manager_; // RWer に関する情報
    Cache cache_; // 他サーバのグラフ情報

    // 再送制御用
    std::vector<std::thread> re_send_threads_;
    uint32_t re_send_count = 0;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline ARWS::ARWS(const std::string& dir_path) {
    // 自サーバ のホスト名
    char hostname_c[128]; // ホスト名
    gethostname(hostname_c, sizeof(hostname_c)); // ホスト名を取得
    hostname_ = hostname_c; // char* から string へ

    // 自サーバ の IP アドレス (NIC 依存)
    const char* ipname;
    if (hostname_[0] == 'a') ipname = "em1"; // abilene
    else if (hostname_[0] == 'e') ipname = "enp2s0"; // espresso
    else ipname = "vlan151"; // giji-4

    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    ifr.ifr_addr.sa_family = AF_INET; // IPv4 の IP アドレスを取得したい
    strncpy(ifr.ifr_name, ipname, IFNAMSIZ-1); // ipname の IP アドレスを取得したい
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    hostip_ = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    hostip_str_ = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    // worker の IP アドレス情報を入手
    std::ifstream reading_file;
    reading_file.open("../graph_data/server.txt", std::ios::in);
    std::string reading_line_buffer;
    while (std::getline(reading_file, reading_line_buffer)) {
        worker_ip_all_.emplace_back(inet_addr(reading_line_buffer.c_str()));
    }
    for (int i = 0; i < worker_ip_all_.size(); i++) {
        if (hostip_ == worker_ip_all_[i]) hostid_ = i;
    }
    // debug
    std::cout << hostid_ << std::endl;

    // グラフファイル読み込み
    graph_.init(dir_path, hostip_str_, hostid_);

    // キャッシュの初期化
    cache_.init();

    // 受信キューの初期化
    // for (int i = 0; i < RECV_PORT; i++) {
    //     message_queue_[10000+i].getSize();
    // }
    message_queue_ = new MessageQueue<char[]>[PROC_MESSAGE];

    // 送信キューの初期化
    // for (uint32_t ip : worker_ip_all_) {
    //     if (ip != hostip_) send_queue_[ip].getSize();
    // }
    send_queue_ = new MessageQueue<RandomWalker>[worker_ip_all_.size()];

    // 全てのスレッドを開始させる 
    start();
}

inline void ARWS::start() {

    std::thread thread_generateRWer(&ARWS::generateRWer, this);

    std::vector<std::thread> threads_sendMessage;
    for (int i = 0; i < worker_ip_all_.size(); i++) {
        if (i == hostid_) continue;
        threads_sendMessage.emplace_back(std::thread(&ARWS::sendMessage, this, i));
    }

    std::vector<std::thread> threads_receiveMessage;
    for (int i = 0; i < RECV_PORT; i++) {
        for (int j = 0; j < RECV_PER_PORT; j++) {
            threads_receiveMessage.emplace_back(std::thread(&ARWS::receiveMessage, this, 10000+i));
        }
    }

    // std::vector<std::thread> threads_procMessage;
    // for (int i = 0; i < RECV_PORT; i++) {
    //     for (int j = 0; j < PROC_MESSAGE_PER_PORT; j++) {
    //         threads_procMessage.emplace_back(std::thread(&ARWS::procMessage, this, 10000+i));
    //     }
    //     // threads_procMessage.emplace_back(std::thread(&ARWS::procMessage, this, 10000+i));
    // }
    // for (int i = 0; i < PROC_MESSAGE; i++) {
    //     threads_procMessage.emplace_back(std::thread(&ARWS::procMessage, this, i));
    // }

    // プログラムを終了させないようにする
    thread_generateRWer.join();
}

inline void ARWS::generateRWer() {
    std::cout << "generateRWer" << std::endl;

    // スレッドごとの乱数生成器
    StdRandNumGenerator* randgen = new StdRandNumGenerator[GENERATE_RWER];

    while (1) {
        // 開始通知を受けるまでロック
        start_flag_.lockWhileFalse();

        // debug
        std::cout << "start" << std::endl;

        uint32_t number_of_RW_execution = RW_config_.getNumberOfRWExecution();
        uint32_t number_of_my_vertices = graph_.getMyVerticesNum();
        std::vector<uint64_t> my_vertices = graph_.getMyVertices();
        uint64_t RWer_num_all = number_of_my_vertices * number_of_RW_execution;

        RW_manager_.init(RWer_num_all);

        // 再送制御用スレッドの立ち上げ
        // re_send_threads_.push_back(std::thread(&ARWS::reSendThread, this));


        // debug
        std::cout << GENERATE_RWER << std::endl;
        uint32_t RWer_id = 0;
        StdRandNumGenerator* randgen = new StdRandNumGenerator[GENERATE_RWER];

        Timer timer;
        #pragma omp parallel private(RWer_id) num_threads(GENERATE_RWER)
        {
            worker_id_t worker_id = omp_get_thread_num();
            StdRandNumGenerator gen = randgen[worker_id];
            RWer_id = worker_id;
            bool sleep_flag = false;

            while (1) {

                vertex_id_t node_id = my_vertices[RWer_id % number_of_my_vertices];

                // 歩数を生成
                uint16_t life = RW_config_.getRWerLife(gen);

                // RWer を生成
                std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(node_id, graph_.getDegree(node_id), RWer_id, hostid_, life));

                // 生成時刻を記録
                RW_manager_.setStartTime(RWer_id);

                // 歩数を記録
                RW_manager_.setRWerLife(RWer_id, life);

                // node_id を記録
                RW_manager_.setNodeId(RWer_id, node_id);

                // RW を実行 
                executeRandomWalk(std::move(RWer_ptr), gen);
                // message_queue_[gen.gen(PROC_MESSAGE)].push(std::move(RWer_ptr));
                
                RWer_id += GENERATE_RWER;
                if (RWer_id >= RWer_num_all) break;
                // if (GENERATE_SLEEP_FLAG && !sleep_flag && (RWer_id * 100) >= (RWer_num_all * RW_STEP_1)) {
                if (GENERATE_SLEEP_FLAG && !sleep_flag && RWer_id >= number_of_my_vertices * RW_STEP_2) {
                    sleep_flag = true;

                    // debug
                    std::cout << "RWer_id: " << RWer_id << ", sleep" << std::endl;

                    std::this_thread::sleep_for(std::chrono::seconds(GENERATE_SLEEP));
                    
                    // debug
                    std::cout << "start" << std::endl;
                }

            }
        }

        std::cout << timer.duration() << std::endl;

        // while (1) {
        //     std::this_thread::sleep_for(std::chrono::seconds(20));
        //     std::cout << timer.duration() << " " << RW_manager_.getEndcnt() << std::endl;
        // }
        
        std::vector<std::thread> threads_procMessage;
        for (int i = 0; i < PROC_MESSAGE; i++) {
            threads_procMessage.emplace_back(std::thread(&ARWS::procMessage, this, i));
        }

        threads_procMessage[0].join();

    }

    delete[] randgen;
}

inline void ARWS::reSendThread() {
    std::cout << "reSendThread" << std::endl;

    bool re_send_flag = false;

    while (1) {
        if (re_send_flag == false && RW_manager_.getEndcnt() > RW_manager_.getRWerAll() * 0.99) {
            re_send_flag = true;
        }

        if (re_send_flag == true) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // debug
    std::cout << "reSend start" << std::endl;

    // 再送制御開始
    uint32_t RWer_all = RW_manager_.getRWerAll();
    std::unordered_set<uint32_t> not_finished_RWer_id;
    double RWer_life_time_sum = 0;
    uint32_t RWer_life_sum = 0;
    for (int RWer_id = 0; RWer_id < RWer_all; RWer_id++) {
        if (RW_manager_.isEnd(RWer_id)) {
            RWer_life_time_sum += RW_manager_.getRWerLifeTime(RWer_id);
            RWer_life_sum += RW_manager_.getRWerLife(RWer_id);
        } else {
            not_finished_RWer_id.insert(RWer_id);
        }
    }
    double time_per_step = RWer_life_time_sum / RWer_life_sum; // 一歩当たりにかかる時間の目安 (ms)

    // debug
    std::cout << "time_per_step: " << time_per_step << std::endl;

    StdRandNumGenerator gen;

    while (not_finished_RWer_id.size()) { // 終了していない RWer がなくなるまで 100 ms ごとに実行

        for (auto it = not_finished_RWer_id.begin(); it != not_finished_RWer_id.end();) {

            if (!RW_manager_.isStart(*it)) { // まだスタートしてない場合

                it++;
                continue;

            } else if (RW_manager_.isEnd(*it)) { // 終了していたら

                it = not_finished_RWer_id.erase(it);

            } else { // まだ終了していない

                re_send_count++;

                vertex_id_t node_id = RW_manager_.getNodeId(*it);
                // RWer を生成
                std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(node_id, graph_.getDegree(node_id), *it, hostip_, RW_manager_.getRWerLife(*it)));

                // 生成時刻を記録
                RW_manager_.setStartTime(*it);

                // RW を実行 
                executeRandomWalk(std::move(RWer_ptr), gen);
                // message_queue_[gen.gen(PROC_MESSAGE)].push(std::move(RWer_ptr));

                // debug
                // std::cout << "resend done" << std::endl;
                it++;
            }

        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


inline void ARWS::executeRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr, StdRandNumGenerator& gen) {

    while (1) {

        // debug
        // std::cout << "executeRandomWalk start" << std::endl;
        // RWer_ptr->printRWer();

        vertex_id_t current_node = RWer_ptr->getCurrentNode(); // 現在頂点

        if (graph_.hasVertex(current_node)) { // 元グラフのデータを参照して RW

            // debug
            // std::cout << "my graph" << std::endl;

            index_t degree = graph_.getDegree(current_node);

            // 現在頂点の次数情報を RWer に入力
            RWer_ptr->setCurrentDegree(degree);
            // current node -> prev node の index を登録
            vertex_id_t prev_node = RWer_ptr->getPrevNode();
            if (prev_node != INF) RWer_ptr->setPrevIndex(graph_.indexOfUV(current_node, prev_node));

            // RW を一歩進める
            if (RWer_ptr->isSended() == true && RWer_ptr->isInputNextIndex() == true) { // 他のサーバから送られてきた RWer

                index_t next_index = RWer_ptr->getNextIndex();
                vertex_id_t next_node = graph_.getNextNode(current_node, next_index, gen);

                RWer_ptr->updateRWer(next_node, graph_.getHostId(next_node), INF, next_index, INF);

            } else if (RWer_ptr->isEnd() || degree == 0) { // 寿命切れ もしくは次数 0 なら終了

                // 終了した RWer の処理 
                endRandomWalk(std::move(RWer_ptr));

                break;

            } else { // ランダムな隣接ノードへ遷移

                index_t next_index = gen.gen(degree);
                vertex_id_t next_node = graph_.getNextNode(current_node, next_index, gen);

                // debug
                // std::cout << "next_index: " << next_index << ", next_node: " << next_node << std::endl; 
                // std::cout << std::endl
                // std::cout << std::endl;
                // std::cout << std::endl;

                RWer_ptr->updateRWer(next_node, graph_.getHostId(next_node), 0, next_index, INF);

            }

        } else { // キャッシュデータを参照して RW

            // debug
            // std::cout << "cache" << std::endl;

            // 現在頂点の次数情報があるか確認
            if (!cache_.hasDegree(current_node)) { // 次数情報がない (元グラフの他サーバ隣接ノードの初期状態)
                // debug
                // std::cout << "motolinsetu" << std::endl; 

                RWer_ptr->inputSendFlag(true);
                send_queue_[graph_.getHostId(current_node)].push(std::move(RWer_ptr));

                break;
            }

            // 次数をキャッシュからコピーして取ってくる
            index_t degree = cache_.getDegree(current_node);

            // 現在頂点の次数情報を RWer に入力
            RWer_ptr->setCurrentDegree(degree);

            // RW を一歩進める
            if (RWer_ptr->isEnd() || degree == 0) { // 寿命切れ もしくは次数 0 なら終了

                // 終了した RWer の処理
                endRandomWalk(std::move(RWer_ptr));

                break;

            } else { // ランダムな隣接ノードへ遷移

                // 0 <= rand_idx < degree をランダム生成
                index_t rand_idx = gen.gen(degree);

                vertex_id_t next_node = cache_.getNextNode(current_node, rand_idx);

                if (next_node == INF) { // index が存在してなかった場合は index とともに送信

                    RWer_ptr->setIndex(rand_idx);
                    RWer_ptr->inputSendFlag(true);
                    send_queue_[cache_.getHostId(current_node)].push(std::move(RWer_ptr));

                    break;

                }

                if (graph_.hasVertex(next_node)) RWer_ptr->updateRWer(next_node, graph_.getHostId(next_node), INF, rand_idx, INF);
                else RWer_ptr->updateRWer(next_node, cache_.getHostId(next_node), INF, rand_idx, INF);
                
            }

        }

    }
}

inline void ARWS::endRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr) {
    // RWer の message_id に DEAD_SEND フラグを入れる
    RWer_ptr->inputMessageID(DEAD_SEND);

    // debug
    // std::cout << "end RWer" << std::endl;
    // RWer_ptr->printRWer();

    // // RWer が通ってきたサーバの集合を入手
    // std::unordered_set<uint64_t> ip_set = RWer_ptr->getHostGroup();

    // // RWer のコピー用の文字列配列
    // char RWer_str[MESSAGE_MAX_LENGTH];
    // RWer_ptr->writeMessage(RWer_str);

    // // それぞれのサーバへ送信 (自分のサーバへは送信せずここで処理)
    // for (uint64_t ip : ip_set) {
    //     std::unique_ptr<RandomWalker> RWer_ptr_cp(new RandomWalker(RWer_str));

    //     if (ip == hostip_) {
    //         checkRWer(std::move(RWer_ptr_cp));
    //     } else {
    //         send_queue_[ip].push(std::move(RWer_ptr_cp));
    //     }
    // }

    if (RWer_ptr->getHostID() == hostid_) {
        if (CHECK_FLAG && RWer_ptr->isSendedAll()) checkRWer(std::move(RWer_ptr));
        else if (MAIN_EX) RW_manager_.setEndTime(RWer_ptr->getRWerID());
    } else {
        send_queue_[RWer_ptr->getHostID()].push(std::move(RWer_ptr));
    }
}

inline void ARWS::checkRWer(std::unique_ptr<RandomWalker>&& RWer_ptr) {
    // debug
    // std::cout << "checkRWer" << std::endl;

    // RWer の起点サーバがここだったら終了時間記録
    if (RWer_ptr->getHostID() == hostid_) {
        // std::cout << "endatstartserver" << std::endl;
        if (MAIN_EX) RW_manager_.setEndTime(RWer_ptr->getRWerID());
    }

    // debug 
    // std::cout << "not host server of RWer" << std::endl;

    // RWer の経路情報をキャッシュに登録
    cache_.addRWer(std::move(RWer_ptr), graph_);
}

inline void ARWS::procMessage(const uint16_t& proc_id) {
    std::cout << "procMessage: " << proc_id << ", " << message_queue_[proc_id].getSize() << std::endl;

    StdRandNumGenerator randgen;

    while (1) {
        // メッセージキューからメッセージを取得
        std::vector<std::unique_ptr<char[]>> message_ptr_vec;
        uint32_t vec_size = message_queue_[proc_id].pop(message_ptr_vec);

        // debug
        // RWer.printRWer();

        for (int i = 0; i < vec_size; i++) {
            // message に入っている RWer の数を確認
            int idx = sizeof(uint8_t);
            uint16_t RWer_count = *(uint16_t*)(message_ptr_vec[i].get() + idx); idx += sizeof(uint16_t);

            for (int j = 0; j < RWer_count; j++) {

                std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(message_ptr_vec[i].get() + idx));
                idx += RWer_ptr->getRWerSize();

                if (RWer_ptr->getMessageID() == DEAD_SEND) { // 終了して送られてきた RWer の処理

                    if (CHECK_FLAG) checkRWer(std::move(RWer_ptr));
                    else if (MAIN_EX) RW_manager_.setEndTime(RWer_ptr->getRWerID());

                } else { // まだ生存している RWer の処理

                    // RW を実行
                    executeRandomWalk(std::move(RWer_ptr), randgen);

                }      
            }
        }

        // // メッセージキューからメッセージを取得
        // std::vector<std::unique_ptr<RandomWalker>> RWer_ptr_vec;
        // uint32_t vec_size = message_queue_[port_num].pop(RWer_ptr_vec);

        // // debug 
        // // std::cout << vec_size << std::endl;

        // bool use_parallel = (vec_size >= OMP_PARALLEL_THRESHOLD);
        // uint32_t progress = 0;
        // #pragma omp parallel num_threads(PROC_MESSAGE_PER_PORT) if (use_parallel)
        // {
        //     worker_id_t worker_id = omp_get_thread_num();
        //     uint32_t next_idx;
        //     while ((next_idx =  __sync_fetch_and_add(&progress, WORK_STEP)) < vec_size) {
        //         uint32_t begin_idx = next_idx;
        //         uint32_t end_idx = std::min(next_idx + WORK_STEP, vec_size);
        //         for (uint32_t idx = begin_idx; idx < end_idx; idx++) {
        //             if (RWer_ptr_vec[idx]->getMessageID() == DEAD_SEND) { // 終了して送られてきた RWer の処理
                        
        //                 if (CHECK_FLAG) checkRWer(std::move(RWer_ptr_vec[idx]));
        //                 else RW_manager_.setEndTime(RWer_ptr_vec[idx]->getRWerID());

        //             } else { // まだ生存している RWer の処理

        //                 // RW を実行
        //                 executeRandomWalk(std::move(RWer_ptr_vec[idx]), randgen[worker_id]);

        //             }                    
        //         }
        //     }
        // }


    }  

}

inline void ARWS::sendMessage(const host_id_t& dst_id) {
    std::cout << "send_message: " << dst_id << std::endl;

    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    } 

    StdRandNumGenerator gen;

    while (1) {
        // メッセージ宣言
        char message[MESSAGE_MAX_LENGTH];

        
        // キューからRWerを取ってきてメッセージに詰める
        uint16_t RWer_count = 0;
        uint32_t now_length = send_queue_[dst_id].pop(message + sizeof(uint8_t) + sizeof(uint16_t), MESSAGE_MAX_LENGTH, RWer_count);

        // メッセージのヘッダ情報を書き込む
        // バージョン: 4bit (0), 
        // メッセージID: 4bit (2),
        // メッセージに含まれるRWerの個数: 16bit
        uint8_t ver_id = 2;
        memcpy(message, &ver_id, sizeof(uint8_t));
        memcpy(message + sizeof(uint8_t), &RWer_count, sizeof(uint16_t));
        now_length += sizeof(uint8_t) + sizeof(uint16_t);

        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(gen.genRandHostId(10000, 10000+RECV_PORT-1)); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = worker_ip_all_[dst_id]; // IPアドレス

        // データ送信
        sendto(sockfd, message, now_length, 0, (struct sockaddr *)&addr, sizeof(addr)); 
    }
}

inline void ARWS::receiveMessage(const uint16_t& port_num) {
    std::cout << "receive_message: " << port_num << std::endl;

    int sockfd = createUdpServerSocket(port_num);

    StdRandNumGenerator gen;

    while (1) {
        // messageを受信
        std::unique_ptr<char[]> message(new char[MESSAGE_MAX_LENGTH]);
        memset(message.get(), 0, MESSAGE_MAX_LENGTH);
        recv(sockfd, message.get(), MESSAGE_MAX_LENGTH, 0);

        uint8_t ver_id = *(uint8_t*)message.get();

        if ((ver_id & MASK_MESSEGEID) == START_EXP) { // 実験開始の合図
            
            uint32_t startmanager_ip = *(uint32_t*)(message.get() + sizeof(ver_id));
            uint32_t num_RWer = *(uint32_t*)(message.get() + sizeof(ver_id) + sizeof(startmanager_ip));

            startmanagerip_ = startmanager_ip;
            RW_config_.setNumberOfRWExecution(num_RWer);

            // debug
            std::cout << "num_RWer = " << num_RWer << std::endl;

            // 実験開始のフラグを立てる
            start_flag_.writeReady(true);

        } else if ((ver_id & MASK_MESSEGEID) == RWERS) { // RWer のメッセージ
            // message_queue_ に入れる
            message_queue_[gen.gen(PROC_MESSAGE)].push(std::move(message));           
            
        } else if ((ver_id & MASK_MESSEGEID) == END_EXP) { // 実験結果を送信

            sendToStartManager();

        } else {
            perror("wrong id");
            exit(1); // 異常終了
        }
    }
}

inline int ARWS::createUdpServerSocket(const uint16_t& port_num) {
    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }

    // アドレスの生成
    struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
    memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
    addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
    addr.sin_port = htons(port_num); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
    addr.sin_addr.s_addr = hostip_; // IPアドレス, inet_addr()関数はアドレスの翻訳

    // ソケット登録
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { // ソケット, アドレスポインタ, アドレスサイズ // エラー処理
        perror("bind");
        exit(1); // 異常終了
    }

    return sockfd;
}

inline int ARWS::createTcpServerSocket(const uint16_t& port_num) {
    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }

    // アドレスの生成
    struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
    memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
    addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
    addr.sin_port = htons(port_num); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
    addr.sin_addr.s_addr = hostip_; // IPアドレス, inet_addr()関数はアドレスの翻訳

    // ソケット登録
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { // ソケット, アドレスポインタ, アドレスサイズ // エラー処理
        perror("bind");
        exit(1); // 異常終了
    } else {
        std::cout << "bind ok" << std::endl;
    }

    // 受信待ち
    if (listen(sockfd, SOMAXCONN) < 0) { // ソケット, キューの最大長 // エラー処理
        perror("listen");
        close(sockfd); // ソケットクローズ
        exit(1); // 異常終了
    } else {
        std::cout << "listen ok" << std::endl;    
    }

    return sockfd;
}

inline void ARWS::sendToStartManager() {
    // 再送用スレッドを停止させる
    for (std::thread& th : re_send_threads_) {
        th.join();
    }
    re_send_threads_.clear();

    // start manager に送信するのは, RW 終了数, 実行時間
    uint32_t end_count = RW_manager_.getEndcnt();
    double execution_time = RW_manager_.getExecutionTime();
    std::cout << "end_count : " << end_count << std::endl;
    std::cout << "execution_time : " << execution_time << std::endl;

    // debug
    std::cout << "RWer_queue_size: " << PROC_MESSAGE << std::endl;
    for (int i = 0; i < PROC_MESSAGE; i++) {
        std::cout << i << ": " << message_queue_[i].getSize() << std::endl;
    }
    std::cout << "send_queue_size: " << worker_ip_all_.size() << std::endl;
    for (int i = 0; i < worker_ip_all_.size(); i++) {
        std::cout << i << ": " << send_queue_[i].getSize() << std::endl;
    }
    std::cout << "re_send_count: " << re_send_count << std::endl;
    std::cout << "my edges num: " << graph_.getEdgeCount() << std::endl;
    std::cout << "cache edges num: " << cache_.getEdgeCount() << std::endl;
    std::cout << "all edges: " << graph_.getEdgeCount() + cache_.getEdgeCount() << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        // ソケットの生成
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) { // エラー処理
            perror("socket");
            exit(1); // 異常終了
        }

        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(9999); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = startmanagerip_; // IPアドレス, inet_addr()関数はアドレスの翻訳

        // ソケット接続要求
        connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)); // ソケット, アドレスポインタ, アドレスサイズ

        // データ送信 (hostip: 4B, end_count: 4B, all_execution_time: 8B)
        char message[MESSAGE_MAX_LENGTH];
        int idx = 0;
        memcpy(message + idx, &hostip_, sizeof(uint32_t)); idx += sizeof(uint32_t);
        memcpy(message + idx, &end_count, sizeof(uint32_t)); idx += sizeof(uint32_t);
        memcpy(message + idx, &execution_time, sizeof(double)); idx += sizeof(double);
        memcpy(message + idx, &re_send_count, sizeof(uint32_t)); idx += sizeof(uint32_t);
        send(sockfd, message, sizeof(message), 0); // 送信

        // ソケットクローズ
        close(sockfd); 
    }

    // RW Manager　のリセット
    // RW_manager_.reset();

    // キャッシュデータのリセット
    // cache_.reset();

    std::cout << "reset ok" << std::endl;

}