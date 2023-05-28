#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <random>
#include <memory>
#include <chrono>
#include <thread>
#include <omp.h>
#include <unordered_set>

#include "graph.hpp"
#include "message_queue.hpp"
#include "start_flag.hpp"
#include "random_walk_config.hpp"
#include "random_walker_manager.hpp"
#include "random_walker.hpp"
#include "measurement.hpp"

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

    // RW を実行する関数
    void executeRandomWalk(RandomWalker& RWer);

    // executeRandomWalk で終了した RWer を処理する関数
    void endRandomWalk(RandomWalker& RWer);

    // 終了した RWer について, 経路情報からグラフデータにキャッシュを登録する関数
    void checkRWer(RandomWalker& RWer);

    // メッセージ処理用の関数 (ポート番号毎)
    void procMessage(const uint16_t& port_num);

    // send_queue から RWer を取ってきて他サーバへ送信する関数 (送信先毎)
    void sendMessage(const uint32_t& dst_ip);

    // RWer を送信する関数
    void sendMessageFunc(RandomWalker& RWer, const uint32_t& dst_ip, int sockfd, const uint16_t& port_num);

    // 他サーバからメッセージを受信し, message_queue に push する関数 (ポート番号毎)
    void receiveMessage(int sockfd, const uint16_t& port_num);

    // IPv4 サーバソケットを生成 (UDP)
    int createUdpServerSocket(const uint16_t& port_num);

    // IPv4 サーバソケットを生成 (TCP)
    int createTcpServerSocket(const uint16_t& port_num);

    // 実験結果を start_manager に送信する関数
    void sendToStartManager();

    // 実験観察者
    void observer();

private :

    std::string hostname_; // 自サーバのホスト名
    uint32_t hostip_; // 自サーバの IP アドレス
    uint32_t startmanagerip_; // StartManager の IP アドレス
    Graph graph_; // グラフデータ
    std::unordered_map<uint16_t, MessageQueue<RandomWalker>> receive_queue_; // ポート番号毎の receive キュー
    std::unordered_map<uint32_t, MessageQueue<RandomWalker>> send_queue_; // 送信先毎の send キュー
    StartFlag start_flag_; // 実験開始の合図に関する情報
    RandomWalkConfig RW_config_; // Random Walk 実行関連の設定
    RandomWalkerManager RW_manager_; // RWer に関する情報
    Measurement measurement_; // 時間計測用

    std::vector<std::thread> observer_thread_; 
    bool observer_flag_ = true;
    bool parallel_flag_ = false;
    bool surviving_over_MAX_div2_flag_ = false;

    // パラメタ (スレッド数)
    const uint32_t SEND_RECV_PORT = 8;
    const uint32_t SEND_PER_PORT = 1;
    const uint32_t RECV_PER_PORT = 1;
    const uint32_t PROC_MESSAGE_PER_PORT = 1;
    const uint32_t GENERATE_RWER = 20;

    // パラメタ (メッセージ長)
    const uint32_t MESSAGE_MAX_LENGTH = 1450;

    // パラメタ (メッセージ終了判定)
    const uint8_t MESSAGE_END = 100;

    // 送信先IP
    std::vector<std::string> worker_ip_all_ = {"10.58.60.3", "10.58.60.5", "10.58.60.6", "10.58.60.7", "10.58.60.8"};
    std::vector<uint32_t> worker_ip_;

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成
    std::uniform_real_distribution<double>  rand_double{0, 1.0}; // 0~1のランダムな値

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

    // グラフファイル読み込み
    graph_.init(dir_path, hostname_, hostip_);

    // 送信先IP
    for (std::string ip : worker_ip_all_) {
        uint32_t ip_int = inet_addr(ip.c_str());
        if (ip_int != hostip_) worker_ip_.push_back(ip_int);
    }

    // キューを初期化
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        receive_queue_[10000+i].getSize();
    }
    // for (int i = 0; i < SEND_RECV_PORT; i++) {
    //     send_queue_[10000+i].getSize();
    // }
    for (uint32_t ip : worker_ip_) {
        send_queue_[ip].getSize();
    }

    // 全てのスレッドを開始させる
    start();
}

inline void ARWS::start() {

    std::thread thread_generateRWer(&ARWS::generateRWer, this);

    std::vector<std::thread> threads_sendMessage;
    // for (int i = 0; i < SEND_RECV_PORT; i++) {
    //     for (int j = 0; j < SEND_PER_PORT; j++) {
    //         threads_sendMessage.emplace_back(std::thread(&ARWS::sendMessage, this, 10000+i));
    //     }
    // }
    for (uint32_t ip : worker_ip_) {
        threads_sendMessage.emplace_back(std::thread(&ARWS::sendMessage, this, ip));
    }

    std::vector<std::thread> threads_receiveMessage;
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        int sockfd = createUdpServerSocket(10000+i);
        for (int j = 0; j < RECV_PER_PORT; j++) {
            threads_receiveMessage.emplace_back(std::thread(&ARWS::receiveMessage, this, sockfd, 10000+i));
        }
    }

    std::vector<std::thread> threads_procMessage;
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        for (int j = 0; j < PROC_MESSAGE_PER_PORT; j++) {
            threads_procMessage.emplace_back(std::thread(&ARWS::procMessage, this, 10000+i));
        }
    }

    // プログラムを終了させないようにする
    thread_generateRWer.join();

}

inline void ARWS::generateRWer() {
    std::cout << "generateRWer" << std::endl;

    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }  

    while (1) {
        // 開始通知を受けるまでロック
        start_flag_.lockWhileFalse();

        std::cout << "start RW!" << std::endl;

        // 並列処理への切り替えのためのフラグの初期化
        parallel_flag_ = false;
        surviving_over_MAX_div2_flag_ = false;

        // observer 起動
        observer_flag_ = true;
        observer_thread_.push_back(std::thread(&ARWS::observer, this));

        uint32_t number_of_RW_execution = RW_config_.getNumberOfRWExecution();
        uint32_t number_of_my_vertices = graph_.getMyVerticesNum();
        std::vector<uint32_t> my_vertices = graph_.getMyVertices();

        RW_manager_.init(number_of_my_vertices * number_of_RW_execution);

        // debug 
        std::cout << "number_of_my_vertices: " << number_of_my_vertices << std::endl;
        std::cout << "number_of_RW_execution: " << number_of_RW_execution << std::endl;
        std::cout << "hasVertex(33): " << graph_.hasVertex(33) << std::endl;
        // std::cout << "hasCacheDegree(34): " << graph_.hasCacheDegree(34) << std::endl;
        // std::cout << "hasCacheDegree(30): " << graph_.hasCacheDegree(30) << std::endl;
        // std::cout << "hasCacheDegree(14): " << graph_.hasCacheDegree(14) << std::endl;
        
        // debug
        std::cout << "thread num: " << omp_get_max_threads() << std::endl;
        uint32_t num_threads = GENERATE_RWER;
        omp_set_num_threads(num_threads);
        std::cout << "thread num: " << omp_get_max_threads() << std::endl;

        measurement_.setStart();

        int now_i = 0;
        for (int i = 0; i < number_of_RW_execution; i++) {

            for (int j = 0; j < number_of_my_vertices; j++) {

                uint32_t node_id = my_vertices[j];

                // debug
                // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                // for (int _ = 0; _ < 5; _++) std::cout << std::endl;

                uint32_t RWer_id = i * number_of_my_vertices + j;

                // debug
                // std::cout << "RWer_id: " << RWer_id << std::endl;
                
                // 生成スピード調整
                RW_manager_.lockWhileOver();

                // RWer を生成
                RandomWalker RWer = RandomWalker(node_id, graph_.getDegree(node_id), RWer_id, hostip_);

                // 生成時刻を記録
                RW_manager_.setStartTime(RWer_id);

                // RW を実行 
                executeRandomWalk(RWer);

                // RWer_id++;

                // std::this_thread::sleep_for(std::chrono::milliseconds(1));

            }

            if (parallel_flag_ == true) {
                now_i = i;
                break;
            }
        }

        // 途中から並列処理
        std::cout << "parallel!!" << std::endl;
        int j;
        #pragma omp parallel for private(j) 
        for (int i = now_i + 1; i < number_of_RW_execution; i++) {

            for (j = 0; j < number_of_my_vertices; j++) {

                uint32_t node_id = my_vertices[j];

                // debug
                // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                // for (int _ = 0; _ < 5; _++) std::cout << std::endl;

                uint32_t RWer_id = i * number_of_my_vertices + j;

                // debug
                // std::cout << "RWer_id: " << RWer_id << std::endl;
                
                // 生成スピード調整
                RW_manager_.lockWhileOver();

                // RWer を生成
                RandomWalker RWer = RandomWalker(node_id, graph_.getDegree(node_id), RWer_id, hostip_);

                // 生成時刻を記録
                RW_manager_.setStartTime(RWer_id);

                // RW を実行 
                executeRandomWalk(RWer);

                // RWer_id++;

                // std::this_thread::sleep_for(std::chrono::milliseconds(1));

            }
        }

        measurement_.setEnd();

        std::cout << measurement_.getExecutionTime() << std::endl;
    }
}

inline void ARWS::executeRandomWalk(RandomWalker& RWer) {

    while (1) {
        
        uint32_t current_node = RWer.getCurrentNode(); // 現在頂点

        if (graph_.hasVertex(current_node)) { // 元グラフのデータを参照して RW

            // debug
            // std::cout << "normalRW, path: ";
            // {
            //     uint32_t path_length = RWer.getPathLength();
            //     uint32_t path[path_length*3]; // 経路情報 : 頂点, HostIP, 次数, 頂点, HostIP, 次数,...
            //     RWer.getPath(path);
            //     for (int i = 0; i < path_length; i++) {
            //         std::cout << path[i*3] << " ";
            //     }
            //     std::cout << std::endl;
            // }

            // 現在頂点の次数情報を RWer に入力
            RWer.inputCurrentDegree(graph_.getDegree(current_node));

            // RW を一歩進める
            if (RWer.getPrevIp() != 0) { // 他のサーバから送られてきた && このサーバでの一歩目のとき

                std::uniform_int_distribution<int> rand_int(0, graph_.getDegree(current_node) - 1);

                uint32_t next_node = graph_.getNextNode(current_node, rand_int(mt));

                int count = 0;
                while (graph_.hasServerCacheEdge(RWer.getPrevIp(), current_node, next_node) && count < 10000) {

                    next_node = graph_.getNextNode(current_node, rand_int(mt));
                    count++;

                }

                RWer.inputPrevIp(0);

                RWer.updateRWer(next_node, graph_.getIP(next_node), 0);

            } else if (rand_double(mt) < RW_config_.getAlpha() || graph_.getDegree(current_node) == 0) { // 確率 α もしくは次数 0 なら終了
                
                // 終了した RWer の処理 
                endRandomWalk(RWer);

                break;

            } else { // ランダムな隣接ノードへ遷移

                std::uniform_int_distribution<int> rand_int(0, graph_.getDegree(current_node) - 1);

                uint32_t next_node = graph_.getNextNode(current_node, rand_int(mt));

                RWer.updateRWer(next_node, graph_.getIP(next_node), 0);
            }

        } else { // キャッシュデータを参照して RW

            // debug
            // std::cout << "cacheRW, path: ";
            // {
            //     uint32_t path_length = RWer.getPathLength();
            //     uint32_t path[path_length*3]; // 経路情報 : 頂点, HostIP, 次数, 頂点, HostIP, 次数,...
            //     RWer.getPath(path);
            //     for (int i = 0; i < path_length; i++) {
            //         std::cout << path[i*3] << " ";
            //     }
            //     std::cout << std::endl;
            // }

            // 現在頂点の次数情報があるか確認
            if (!graph_.hasCacheDegree(current_node)) { // 次数情報がない (元グラフの他サーバ隣接ノード)
                // debug
                // std::cout << "motolinsetu" << std::endl; 

                send_queue_[graph_.getIP(current_node)].push(RWer);

                break;

            } 

            // debug
            // std::cout << "cacheRW2, current_node: " << current_node << std::endl;
        
            // 必要な情報 (次数, 隣接リスト) をキャッシュからコピーして取ってくる (ロック削減のため)
            uint32_t degree = graph_.getCacheDegree(current_node);
            // std::vector<uint32_t> adjacency_list = graph_.getCacheAdjacencyList(current_node);

            // 現在頂点の次数情報を RWer に入力
            RWer.inputCurrentDegree(degree);

            // RW を一歩進める
            if (rand_double(mt) < RW_config_.getAlpha() || degree == 0) { // 確率 α もしくは次数 0 なら終了
                
                // 終了した RWer の処理 
                endRandomWalk(RWer);

                break;

            } else { // ランダムな隣接ノードへ遷移

                // 0 <= rand_idx < degree をランダム生成
                std::uniform_int_distribution<int> rand_int(0, degree - 1);
                uint32_t rand_idx = rand_int(mt);

                if (rand_idx < graph_.getCacheAdjacencyListSize(current_node)) { // 隣接リストから遷移先を選択

                    uint32_t next_node = graph_.getCacheNextNode(current_node, rand_idx);
                    RWer.updateRWer(next_node, graph_.getCacheIP(next_node), 0);

                } else { // 遷移先が不明のため送信

                    send_queue_[graph_.getCacheIP(current_node)].push(RWer);

                    break;

                }    
            }

        }
    } 
}

inline void ARWS::endRandomWalk(RandomWalker& RWer) {
    // debug
    // std::cout << "endRandomWalk" << std::endl;

    // RWer の終了フラグを立てる
    RWer.endRWer();

    if (RWer.getSendFlag() == 0) { // 送信が発生していない RWer は他のサーバに伝える必要がない

        RW_manager_.setEndTime(RWer.getRWerId());

    } else {
        // RWer が通ってきたサーバの集合を入手
        std::unordered_set<uint32_t> ip_set = RWer.getServerGroup();

        // それぞれのサーバへ送信 (自分のサーバへは送信せずここで処理)
        for (uint32_t ip : ip_set) {
            if (ip == hostip_) {
                checkRWer(RWer);
            } else {
                send_queue_[ip].push(RWer);
            }
        }
    }

    
}

inline void ARWS::checkRWer(RandomWalker& RWer) {
    // RWer の起点サーバがここだったら終了時間記録
    if (RWer.getHostip() == hostip_) {
        RW_manager_.setEndTime(RWer.getRWerId());
    }

    // RWer が通ってきたサーバの集合を入手
    std::unordered_set<uint32_t> ip_set = RWer.getServerGroup();

    // RWer の経路情報を見てグラフデータをキャッシュに登録
    uint32_t path_length = RWer.getPathLength();
    uint32_t path[path_length*3]; // 経路情報 : 頂点, HostIP, 次数, 頂点, HostIP, 次数,...
    RWer.getPath(path);
    for (int i = 0; i < path_length-1; i++) {
        // 両方向エッジをキャッシュに追加
        graph_.addEdgeToCache(path[i*3], path[i*3+1], path[i*3+2], path[(i+1)*3], path[(i+1)*3 + 1], path[(i+1)*3 + 2], ip_set);
        graph_.addEdgeToCache(path[(i+1)*3], path[(i+1)*3 + 1], path[(i+1)*3 + 2], path[i*3], path[i*3+1], path[i*3+2], ip_set);
    }
}

inline void ARWS::procMessage(const uint16_t& port_num) {
    std::cout << "procMessage: " << port_num << std::endl;

    while (1) {
        // メッセージキューからメッセージを取得
        RandomWalker RWer = receive_queue_[port_num].pop();

        // debug
        // RWer.printRWer();

        if (RWer.getEndFlag() == 1) { // 終了した RWer の処理

            checkRWer(RWer);

        } else { // まだ生存している RWer の処理

            // RW を実行
            executeRandomWalk(RWer);

        }

    }  
}

inline void ARWS::sendMessage(const uint32_t& dst_ip) {
    std::cout << "send_message: " << dst_ip << std::endl;

    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }  

    // debug 
    // std::cout << 100 << std::endl;

    while (1) {
        // メッセージ宣言
        char message[MESSAGE_MAX_LENGTH + sizeof(MESSAGE_END)];
        // std::unique_ptr<char[]> message(new char[MESSAGE_MAX_LENGTH + MESSAGE_END]);

        // debug 
        // std::cout << 100 << std::endl;

        // キューからRWerを取ってきてメッセージに詰める
        uint32_t now_length = send_queue_[dst_ip].pop(message, MESSAGE_MAX_LENGTH, hostip_);

        // メッセージ終端の目印を書き込む
        memcpy(message + now_length, &MESSAGE_END, sizeof(MESSAGE_END));
        now_length += sizeof(MESSAGE_END);

        // ポート番号をランダムに生成
        std::uniform_int_distribution<int> rand_port(10000, 10000+SEND_RECV_PORT-1);

        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(rand_port(mt)); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = dst_ip; // IPアドレス

        // データ送信
        sendto(sockfd, message, now_length, 0, (struct sockaddr *)&addr, sizeof(addr)); 
    }
}

inline void ARWS::sendMessageFunc(RandomWalker& RWer, const uint32_t& dst_ip, int sockfd, const uint16_t& port_num) {
    // アドレスの生成
    struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
    memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
    addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
    addr.sin_port = htons(port_num); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
    addr.sin_addr.s_addr = dst_ip; // IPアドレス

    // データ送信
    sendto(sockfd, &RWer, MESSAGE_MAX_LENGTH, 0, (struct sockaddr *)&addr, sizeof(addr)); 
}

inline void ARWS::receiveMessage(int sockfd, const uint16_t& port_num) {
    std::cout << "receive_message: " << port_num << std::endl;

    while (1) {
        // messageを受信
        std::unique_ptr<char[]> message(new char[MESSAGE_MAX_LENGTH + sizeof(MESSAGE_END)]);
        memset(message.get(), 0, MESSAGE_MAX_LENGTH + sizeof(MESSAGE_END));
        recv(sockfd, message.get(), MESSAGE_MAX_LENGTH + sizeof(MESSAGE_END), 0);

        uint8_t* message_id = (uint8_t*)(message.get());

        if (*message_id == 1) { // 実験開始の合図

            uint32_t* ip = (uint32_t*)(message.get() + sizeof(uint32_t));
            uint32_t* num_RWer = (uint32_t*)(message.get() + sizeof(uint32_t) + sizeof(uint32_t));

            startmanagerip_ = *ip;
            RW_config_.setNumberOfRWExecution(*num_RWer);

            // debug
            std::cout << *num_RWer << std::endl;

            // 実験開始のフラグを立てる
            start_flag_.writeReady(true);

        } else if (*message_id == 2) { // RWer のメッセージ
            // debug
            // std::uniform_int_distribution<int> rand_int(0, 1000);
            // std::this_thread::sleep_for(std::chrono::milliseconds(rand_int(mt)));
            // std::cout << "message_id: 2" << std::endl;

            // RWer を一つずつ分ける
            uint32_t stream_length = 0;

            while (1) {
                if (*message_id == MESSAGE_END) break;

                // std::unique_ptr<RandomWalker> RWer_ptr((RandomWalker*)(message.get() + stream_length));
                RandomWalker RWer = *((RandomWalker*)(message.get() + stream_length));

                uint32_t RWer_data_length = RWer.getSize();

                // メッセージキューに push
                receive_queue_[port_num].push(RWer);

                stream_length += RWer_data_length;
                message_id = (uint8_t*)(message.get() + stream_length); // 次の message_id を特定
            }

        } else if (*message_id == 3) { // 実験結果を送信

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
    // observer 停止
    observer_flag_ = false;
    for (std::thread &th : observer_thread_) {
        th.join();
    }
    observer_thread_.clear();

    // start manager に送信するのは, RW 終了数, 実行時間
    uint32_t end_count = RW_manager_.getEndcnt();
    double execution_time = RW_manager_.getExecutionTime();
    std::cout << "end_count : " << end_count << std::endl;
    std::cout << "execution_time : " << execution_time << std::endl;

    // debug
    std::cout << "message_queue_size: " << receive_queue_.size() << std::endl;
    for (auto& [port, q] : receive_queue_) {
        std::cout << port << ": " << q.getSize() << std::endl;
    }
    std::cout << "send_queue_size: " << send_queue_.size() << std::endl;
    for (auto& [ip, q] : send_queue_) {
        std::cout << ip << ": " << q.getSize() << std::endl;   
    }
    std::cout << "my edges num: " << graph_.getMyEdgeCount() << std::endl;
    std::cout << "cache edges num: " << graph_.getCacheEdgeCount() << std::endl;
    std::cout << "all edges: " << graph_.getMyEdgeCount() + graph_.getCacheEdgeCount() << std::endl;

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
        char message[1024];
        memcpy(message, &hostip_, sizeof(uint32_t));
        memcpy(message + sizeof(uint32_t), &end_count, sizeof(uint32_t));
        memcpy(message + sizeof(uint32_t) + sizeof(uint32_t), &execution_time, sizeof(double));
        send(sockfd, message, sizeof(message), 0); // 送信

        // ソケットクローズ
        close(sockfd); 
    }

    // RW Manager　のリセット
    RW_manager_.reset();

    // キャッシュデータのリセット
    graph_.reset();

    std::cout << "reset ok" << std::endl;

    std::cout << "my edges num: " << graph_.getMyEdgeCount() << std::endl;
    std::cout << "cache edges num: " << graph_.getCacheEdgeCount() << std::endl;
    std::cout << "all edges: " << graph_.getMyEdgeCount() + graph_.getCacheEdgeCount() << std::endl;
}

inline void ARWS::observer() {
    uint32_t max_threads_num = omp_get_max_threads();

    while (observer_flag_) {
        if (RW_manager_.getSurvivingRWerNum() > RW_manager_.getMaxSurvivingRwer() / 2) {
            surviving_over_MAX_div2_flag_ = true;
        }

        if (parallel_flag_ == false && surviving_over_MAX_div2_flag_ && RW_manager_.getSurvivingRWerNum() < max_threads_num) {
            parallel_flag_ = true;

            // debug
            std::cout << "all edges: " << graph_.getMyEdgeCount() + graph_.getCacheEdgeCount() << std::endl;

            // observer_flag_ = false;
        }
        // measurement_.setEnd();
        // if (parallel_flag_ == false && measurement_.getExecutionTime() > 10) {
        //     parallel_flag_ = true;
        //     std::cout << "parallel!" << std::endl;
        // }

        RW_manager_.printSurvivingRWerNum();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
