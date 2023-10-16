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

#include "graph.hpp"
#include "message_queue.hpp"
#include "random_walker.hpp"
#include "start_flag.hpp"
#include "random_walk_config.hpp"
#include "random_walker_manager.hpp"
#include "measurement.hpp"
#include "cache.hpp"
#include "param.hpp"

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
    void executeRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr);

    // executeRandomWalk で終了した RWer を処理する関数
    void endRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr);

    // 終了した RWer について, 経路情報からグラフデータにキャッシュを登録する関数
    void checkRWer(std::unique_ptr<RandomWalker>&& RWer_ptr);

    // メッセージ処理用の関数 (ポート番号毎)
    void procMessage(const uint16_t& port_num);

    // send_queue から RWer を取ってきて他サーバへ送信する関数 (送信先毎)
    void sendMessage(const uint32_t& dst_ip);

    // 他サーバからメッセージを受信し, message_queue に push する関数 (ポート番号毎)
    void receiveMessage(int sockfd, const uint16_t& port_num);

    // IPv4 サーバソケットを生成 (UDP)
    int createUdpServerSocket(const uint16_t& port_num);

    // IPv4 サーバソケットを生成 (TCP)
    int createTcpServerSocket(const uint16_t& port_num);

    // 実験結果を start_manager に送信する関数
    void sendToStartManager();
    

private :

    std::string hostname_; // 自サーバのホスト名
    uint32_t hostip_; // 自サーバの IP アドレス
    uint32_t startmanagerip_; // StartManager の IP アドレス
    Graph graph_; // グラフデータ
    std::vector<uint32_t> worker_ip_all_;
    std::unordered_map<uint16_t, MessageQueue<RandomWalker>> receive_queue_; // ポート番号毎の receive キュー
    std::unordered_map<uint32_t, MessageQueue<RandomWalker>> send_queue_; // 送信先毎の send キュー
    StartFlag start_flag_; // 実験開始の合図に関する情報
    RandomWalkConfig RW_config_; // Random Walk 実行関連の設定
    RandomWalkerManager RW_manager_; // RWer に関する情報
    Measurement measurement_; // 時間計測用
    Cache cache_; // 他サーバのグラフ情報

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成

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

    // worker の IP アドレス情報を入手
    std::ifstream reading_file;
    reading_file.open("../graph_data/server.txt", std::ios::in);
    std::string reading_line_buffer;
    while (std::getline(reading_file, reading_line_buffer)) {
        worker_ip_all_.emplace_back(inet_addr(reading_line_buffer.c_str()));
    }

    // グラフファイル読み込み
    graph_.init(dir_path, hostname_, hostip_, worker_ip_all_);

    // 受信キューの初期化
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        receive_queue_[10000+i].getSize();
    }

    // 送信キューの初期化
    for (uint32_t ip : worker_ip_all_) {
        if (ip != hostip_) send_queue_[ip].getSize();
    }

    // 全てのスレッドを開始させる 
    start();
}

inline void ARWS::start() {

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

        uint32_t number_of_RW_execution = RW_config_.getNumberOfRWExecution();
        uint32_t number_of_my_vertices = graph_.getMyVerticesNum();
        std::vector<uint32_t> my_vertices = graph_.getMyVertices();

        RW_manager_.init(number_of_my_vertices * number_of_RW_execution);

        measurement_.setStart();

        uint32_t num_threads = GENERATE_RWER;
        omp_set_num_threads(num_threads);
        int j;
        #pragma omp parallel for private(j) 
        for (int i = 0; i < number_of_RW_execution; i++) {

            for (j = 0; j < number_of_my_vertices; j++) {

                uint32_t node_id = my_vertices[j];

                uint32_t RWer_id = i * number_of_my_vertices + j;

                // 生成スピード調整
                RW_manager_.lockWhileOver();

                // RWer を生成
                // RandomWalker RWer = RandomWalker(node_id, graph_.getDegree(node_id), RWer_id, hostip_, RW_config_.getRWerLife());
                std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(node_id, graph_.getDegree(node_id), RWer_id, hostip_, RW_config_.getRWerLife()));

                // 生成時刻を記録
                RW_manager_.setStartTime(RWer_id);

                // RW を実行 
                executeRandomWalk(std::move(RWer_ptr));

            }
        }

        measurement_.setEnd();

        std::cout << measurement_.getExecutionTime() << std::endl;
    }
}

inline void ARWS::executeRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr) {

    while (1) {

        uint32_t current_node = RWer_ptr->getCurrentNode(); // 現在頂点

        if (graph_.hasVertex(current_node)) { // 元グラフのデータを参照して RW

            // 現在頂点の次数情報を RWer に入力
            RWer_ptr->setCurrentDegree(graph_.getDegree(current_node));

            // RW を一歩進める
            if (RWer_ptr->isSended() == true) { // 他のサーバから送られてきた RWer

                // current node -> prev node の index を登録
                RWer_ptr->setPrevIndex(graph_.indexOfUV(current_node, RWer_ptr->getPrevNode()));

                uint32_t next_node = graph_.getNextNode(current_node, RWer_ptr->getNextIndex());

                RWer_ptr->updateRWer(next_node, graph_.getIP(next_node), 0, RWer_ptr->getNextIndex(), graph_.indexOfUV(next_node, current_node));

            } else if (RWer_ptr->isEnd() || graph_.getDegree(current_node) == 0) { // 寿命切れ もしくは次数 0 なら終了

                // 終了した RWer の処理 
                endRandomWalk(std::move(RWer_ptr));

                break;

            } else { // ランダムな隣接ノードへ遷移

                std::uniform_int_distribution<int> rand_int(0, graph_.getDegree(current_node) - 1);

                uint32_t next_index = rand_int(mt);
                uint32_t next_node = graph_.getNextNode(current_node, next_index);

                RWer_ptr->updateRWer(next_node, graph_.getIP(next_node), 0, next_index, graph_.indexOfUV(next_node, current_node));

            }

        } else { // キャッシュデータを参照して RW

            // 現在頂点の次数情報があるか確認
            if (!cache_.hasDegree(current_node)) { // 次数情報がない (元グラフの他サーバ隣接ノードの初期状態)
                // debug
                // std::cout << "motolinsetu" << std::endl; 

                send_queue_[graph_.getIP(current_node)].push(std::move(RWer_ptr));

                break;
            }

            // 次数をキャッシュからコピーして取ってくる (ロック削減のため)
            uint32_t degree = cache_.getDegree(current_node);

            // 現在頂点の次数情報を RWer に入力
            RWer_ptr->setCurrentDegree(degree);

            // RW を一歩進める
            if (RWer_ptr->isEnd() || degree == 0) { // 寿命切れ もしくは次数 0 なら終了

                // 終了した RWer の処理
                endRandomWalk(std::move(RWer_ptr));

                break;

            } else { // ランダムな隣接ノードへ遷移

                // 0 <= rand_idx < degree をランダム生成
                std::uniform_int_distribution<int> rand_int(0, degree - 1);
                uint32_t rand_idx = rand_int(mt);

                uint32_t next_node = cache_.getNextNode(current_node, rand_idx);

                if (next_node == INF) { // index が存在してなかった場合は index とともに送信

                    RWer_ptr->setIndex(rand_idx);
                    RWer_ptr->inputSendFlag(true);
                    send_queue_[cache_.getIP(current_node)].push(std::move(RWer_ptr));

                    break;

                }

                RWer_ptr->updateRWer(next_node, graph_.getIP(next_node), 0, rand_idx, INF);

            }

        }

    }
}

inline void ARWS::endRandomWalk(std::unique_ptr<RandomWalker>&& RWer_ptr) {
    // RWer が通ってきたサーバの集合を入手
    std::unordered_set<uint64_t> ip_set = RWer_ptr->getHostGroup();

    // それぞれのサーバへ送信 (自分のサーバへは送信せずここで処理)
    for (uint64_t ip : ip_set) {
        if (ip == hostip_) {
            checkRWer(std::move(RWer_ptr));
        } else {
            send_queue_[ip].push(std::move(RWer_ptr));
        }
    }
}

inline void ARWS::checkRWer(std::unique_ptr<RandomWalker>&& RWer_ptr) {
    // RWer の起点サーバがここだったら終了時間記録
    if (RWer_ptr->getHostID() == hostip_) {
        RW_manager_.setEndTime(RWer_ptr->getRWerID());
    }

    // RWer の経路情報をキャッシュに登録
    cache_.addRWer(std::move(RWer_ptr), graph_);
}

inline void ARWS::procMessage(const uint16_t& port_num) {
    std::cout << "procMessage: " << port_num << std::endl;

    while (1) {
        // メッセージキューからメッセージを取得
        std::unique_ptr<RandomWalker> RWer_ptr = receive_queue_[port_num].pop();

        // debug
        // RWer.printRWer();

        if (RWer_ptr->isEnd()) { // 終了した RWer の処理

            checkRWer(std::move(RWer_ptr));

        } else { // まだ生存している RWer の処理

            // RW を実行
            executeRandomWalk(std::move(RWer_ptr));

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

    while (1) {
        // メッセージ宣言
        char message[MESSAGE_MAX_LENGTH];

        
        // キューからRWerを取ってきてメッセージに詰める
        uint16_t RWer_count = 0;
        uint32_t now_length = send_queue_[dst_ip].pop(message + sizeof(uint8_t) + sizeof(uint16_t), MESSAGE_MAX_LENGTH, hostip_, RWer_count);

        // メッセージのヘッダ情報を書き込む
        // バージョン: 4bit (0), 
        // メッセージID: 4bit (2),
        // メッセージに含まれるRWerの個数: 16bit
        uint8_t ver_id = 2;
        memcpy(message, &ver_id, sizeof(uint8_t));
        memcpy(message + sizeof(uint8_t), &RWer_count, sizeof(uint16_t));
        now_length += sizeof(uint8_t) + sizeof(uint16_t);

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

inline void ARWS::receiveMessage(int sockfd, const uint16_t& port_num) {
    std::cout << "receive_message: " << port_num << std::endl;

    while (1) {
        // messageを受信
        char message[MESSAGE_MAX_LENGTH];
        memset(message, 0, MESSAGE_MAX_LENGTH);
        recv(sockfd, message, MESSAGE_MAX_LENGTH, 0);

        uint8_t ver_id = *(uint8_t*)message;

        if (ver_id & MASK_MESSEGEID == 3) { // 実験開始の合図
            
            uint32_t* ip = (uint32_t*)(message + sizeof(uint32_t));
            uint32_t* num_RWer = (uint32_t*)(message + sizeof(uint32_t) + sizeof(uint32_t));

            startmanagerip_ = *ip;
            RW_config_.setNumberOfRWExecution(*num_RWer);

            // debug
            std::cout << *num_RWer << std::endl;

            // 実験開始のフラグを立てる
            start_flag_.writeReady(true);

        } else if (ver_id & MASK_MESSEGEID == 2) { // RWer のメッセージ
            // message に入っている RWer の数を確認
            int idx = sizeof(uint8_t);
            uint16_t RWer_count = *(uint16_t*)(message + idx); idx += sizeof(uint16_t);

            for (int i = 0; i < RWer_count; i++) {
                std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(message + idx));
                idx += RWer_ptr->getRWerSize();
                // メッセージキューに push
                receive_queue_[port_num].push(std::move(RWer_ptr));
            }
            
        } else if (ver_id & MASK_MESSEGEID == 4) { // 実験結果を送信

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
    // cache_.reset();

    std::cout << "reset ok" << std::endl;

}