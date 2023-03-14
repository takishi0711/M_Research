#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <random>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <omp.h>
#include <thread>
#include <memory>
#include <chrono>
#include <unordered_set>

#include "graph.hpp"
#include "random_walk_config.hpp"
#include "message_queue.hpp"
#include "start_flag.hpp"
#include "random_walker_manager.hpp"
#include "random_walker.hpp"
#include "message.hpp"
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

    // メッセージ処理用の関数 (ポート番号毎)
    void procMessage(const uint16_t port_num);

    // send_queue から RWer を取ってきて他サーバへ送信する関数 (ポート番号毎)
    void sendMessage(const uint16_t port_num);

    // IPv4 サーバソケットを生成 (UDP)
    int createUdpServerSocket(const uint16_t port_num);

    // IPv4 サーバソケットを生成 (TCP)
    int createTcpServerSocket(const uint16_t port_num);

    // 他サーバからメッセージを受信し, message_queue に push する関数 (ポート番号毎)
    void receiveMessage(int sockfd, const uint16_t port_num);

    // 実験結果を start_manager に送信する関数
    void sendToStartManager();

private :

    std::string hostname_; // 自サーバのホスト名
    std::string hostip_; // 自サーバの IP アドレス
    std::string startmanagerip_; // StartManager の IP アドレス
    Graph graph_; // グラフデータ
    RandomWalkConfig RW_config_; // Random Walk 実行関連の設定
    std::unordered_map<uint16_t, MessageQueue> receive_queue_; // ポート番号毎の receive キュー
    std::unordered_map<uint16_t, MessageQueue> send_queue_; // ポート番号毎の send キュー
    RandomWalkerManager RW_manager_; // RWer に関する情報
    StartFlag start_flag_; // 実験開始の合図に関する情報
    Measurement measurement_; // 時間計測用
    std::vector<std::string> worker_ip_ = {"10.58.60.3", "10.58.60.5", "10.58.60.6", "10.58.60.7", "10.58.60.8"}; // 実験で使う通信先 IP アドレス
    std::unordered_map<std::string, char> ip_id = {{"10.58.60.3", '0'}, {"10.58.60.5", '1'}, {"10.58.60.6", '2'}, {"10.58.60.7", '3'}, {"10.58.60.8", '4'}};

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成
    std::uniform_real_distribution<double>  rand_double{0, 1.0}; // 0~1のランダムな値

    // パラメタ (スレッド数)
    const uint32_t SEND_RECV_PORT = 14;
    const uint32_t RECV_PER_PORT = 1;
    const uint32_t PROC_MESSAGE_PER_PORT = 1;
    const uint32_t SEND_PER_PORT = 1;
    const uint32_t GENERATE_RWER = 4;

    // パラメタ (メッセージ長)
    const size_t MESSAGE_LENGTH = 250;

}; 

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline ARWS::ARWS(const std::string& dir_path) {
    // 自サーバ のホスト名
    char hostname_c[128]; // ホスト名
    gethostname(hostname_c, sizeof(hostname_c)); // ホスト名を取得
    hostname_ = hostname_c; // char* から string へ

    // 自サーバ の IP アドレス
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
    hostip_ = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr); // char* から string へ

    // グラフファイル読み込み
    graph_.init(dir_path, hostname_, hostip_);

    // キューを初期化
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        receive_queue_[10000+i].getSize();
    }
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        send_queue_[10000+i].getSize();
    }

    // 全てのスレッドを開始させる
    start();  
}

inline void ARWS::start() {
    // スレッドを開始させる
    std::thread thread_generateRWer(&ARWS::generateRWer, this);

    std::vector<std::thread> threads_sendMessage;
    for (int i = 0; i < SEND_RECV_PORT; i++) {
        for (int j = 0; j < SEND_PER_PORT; j++) {
            threads_sendMessage.emplace_back(std::thread(&ARWS::sendMessage, this, 10000+i));
        }
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
    while (1) {
        // 開始通知を受けるまでロック
        start_flag_.lockWhileFalse();

        uint32_t number_of_RW_execution = RW_config_.getNumberOfRWExecution();
        uint32_t number_of_my_vertices = graph_.getMyVerticesNum();
        std::vector<uint32_t> my_vertices = graph_.getMyVertices();

        RW_manager_.init(number_of_my_vertices * number_of_RW_execution);

        uint32_t num_threads = GENERATE_RWER;
        // omp_set_num_threads(num_threads);

        measurement_.setStart();

        int j;
        // #pragma omp parallel for private(j) 
        for (int i = 0; i < number_of_my_vertices; i++) {

            uint32_t node_id = my_vertices[i];
            // uint32_t worker_id = omp_get_thread_num();

            for (j = 0; j < number_of_RW_execution; j++) {

                uint32_t RWer_id = i * number_of_RW_execution + j;

                // // デバッグ
                // std::cout << RWer_id << std::endl;

                // RWer を生成
                RandomWalker* RWer = new RandomWalker(node_id, RWer_id, hostip_);

                // 生成時刻を記録
                RW_manager_.setStartTime(RWer_id);

                // RW を実行 
                executeRandomWalk(*RWer);

                delete RWer;

                // std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }

        measurement_.setEnd();

        std::cout << measurement_.getExecutionTime() << std::endl;

    }
}

inline void ARWS::executeRandomWalk(RandomWalker& RWer) {
    while (1) {
        // 現在ノードの隣接ノード集合を入手
        std::vector<uint32_t> adjacency_vertices = graph_.getAdjacencyVertices(RWer.getCurrentNode());

        // 次数
        uint32_t degree = adjacency_vertices.size();

        // RW を一歩進める
        if (rand_double(mt) < RW_config_.getAlpha() || degree == 0) { // 確率 α もしくは次数 0 なら終了

            RWer.endRWer();

            if (RWer.getHostip() == hostip_) { // もし終了した RWer の起点サーバが今いるサーバである場合, 終了した RWer をこの場で処理

                RW_manager_.setEndTime(RWer.getRWerId());

            } else { // そうでないならメッセージを生成し, send_queue に push

                std::unique_ptr<char[]> message(new char[MESSAGE_LENGTH]);
                message[0] = '2';
                message[1] = ip_id[RWer.getHostip()];
                int message_index = 2;
                RWer.serialize(message.get(), message_index);
                std::uniform_int_distribution<int> rand_port(10000, 10000+SEND_RECV_PORT-1);
                send_queue_[rand_port(mt)].push(std::move(message));

            }
            
            break;

        } else { // 確率 1-α でランダムな隣接ノードへ遷移

            // ランダムな隣接ノードへ遷移
            std::uniform_int_distribution<int> rand_int(0, degree-1);
            uint32_t next_node = adjacency_vertices[rand_int(mt)];
            RWer.updateRWer(next_node);
            std::string next_node_ip = graph_.getIP(next_node);

            // 遷移先ノードの持ち主が自サーバか他サーバかで分類
            if (next_node_ip == hostip_) { // 自サーバへの遷移

                continue;

            } else { // 他サーバへの遷移

                std::unique_ptr<char[]> message(new char[MESSAGE_LENGTH]);
                message[0] = '2';
                message[1] = ip_id[next_node_ip];
                int message_index = 2;
                RWer.serialize(message.get(), message_index);
                std::uniform_int_distribution<int> rand_port(10000, 10000+SEND_RECV_PORT-1);
                send_queue_[rand_port(mt)].push(std::move(message));

                break;

            }
        }
    }
}

inline void ARWS::procMessage(const uint16_t port_num) {
    std::cout << "procMessage: " << port_num << std::endl;
    while (1) {
        // メッセージキューからメッセージを取得
        std::unique_ptr<char[]> message = receive_queue_[port_num].pop();

        // // デバッグ
        // std::cout << port_num << std::endl;
        // std::cout << message.get() << std::endl;

        char message_id = message[0];
        

        if (message_id == '1') { // 実験開始の合図
            // メッセージをchar*型からstring型に変換
            std::string message_str = message.get(); 

            message_str = message_str.substr(1);

            std::vector<std::string> words; // IP アドレス, RW 実行回数
            std::stringstream sstream(message_str);
            std::string word;
            while (std::getline(sstream, word, ',')) { // カンマ区切りでwordを取り出す
                words.push_back(word);
            }

            // StartManager の IP アドレス
            std::string ip;
            for (int i = 0; i < 4; i++) {
                ip += std::to_string(int(words[0][i]));
                ip += '.';
            }
            ip.pop_back();
            startmanagerip_ = ip;

            // RW 実行回数を格納
            uint32_t num_RWer = std::stoi(words[1]);
            RW_config_.setNumberOfRWExecution(num_RWer);

            // 実験開始のフラグを立てる
            start_flag_.writeReady(true);

        } else if (message_id == '2') { // RWer のメッセージ
            // // デバッグ
            // std::cout << "message_id: 2" << std::endl;

            // RWer へデシリアライズ
            int message_index = 2;
            RandomWalker* RWer = new RandomWalker(message.get(), message_index);

            // // デバッグ
            // std::cout << "RWer: " << RWer.getHostip() << std::endl;

            if (RWer->getEndFlag() == true) { // 終了した RWer の処理
                
                RW_manager_.setEndTime(RWer->getRWerId());

                // // デバッグ
                // std::cout << "end RWer" << std::endl;

            } else { // まだ生存している RWer の処理
                // RW を実行
                executeRandomWalk(*RWer);
            }

            delete RWer;

        } else if (message_id == '3') { // 実験結果を送信

            sendToStartManager();

        } else {
            perror("wrong id");
            exit(1); // 異常終了
        }
    }
}

inline void ARWS::sendMessage(const uint16_t port_num) {
    std::cout << "send_message: " << port_num << std::endl;

    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }  

    while (1) {
        // send_queue からメッセージを入手
        std::unique_ptr<char[]> message = send_queue_[port_num].pop();

        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(port_num); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = inet_addr(worker_ip_[message[1]-'0'].c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

        // データ送信
        sendto(sockfd, message.get(), MESSAGE_LENGTH, 0, (struct sockaddr *)&addr, sizeof(addr)); 

        // // デバッグ
        // std::cout << "send: " << send_ip << std::endl;
    }
}

inline int ARWS::createUdpServerSocket(const uint16_t port_num) {
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
    addr.sin_addr.s_addr = inet_addr(hostip_.c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

    // ソケット登録
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { // ソケット, アドレスポインタ, アドレスサイズ // エラー処理
        perror("bind");
        exit(1); // 異常終了
    }

    return sockfd;
}

inline int ARWS::createTcpServerSocket(const uint16_t port_num) {
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
    addr.sin_addr.s_addr = inet_addr(hostip_.c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

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

inline void ARWS::receiveMessage(int sockfd, const uint16_t port_num) {
    std::cout << "receive_message: " << port_num << std::endl;

    while (1) {
        // messageを受信
        std::unique_ptr<char[]> message(new char[MESSAGE_LENGTH]);
        memset(message.get(), 0, MESSAGE_LENGTH);
        recv(sockfd, message.get(), MESSAGE_LENGTH, 0);

        // // デバッグ
        // std::cout << buffer << std::endl;

        // // デバッグ
        // std::cout << message.get() << std::endl;

        // メッセージキューに push
        receive_queue_[port_num].push(std::move(message));

        // // デバッグ
        // std::cout << "recv: " << port_num << std::endl;
    }
}

inline void ARWS::sendToStartManager() {
    // start manager に送信するのは, RW 終了数, 実行時間
    uint32_t end_count = RW_manager_.getEndcnt();
    double execution_time = RW_manager_.getExecutionTime();
    std::cout << "end_count : " << end_count << std::endl;
    std::cout << "execution_time : " << execution_time << std::endl;

    // デバッグ
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
        addr.sin_addr.s_addr = inet_addr(startmanagerip_.c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

        // ソケット接続要求
        connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)); // ソケット, アドレスポインタ, アドレスサイズ

        // データ送信 (hostname, end_count, execution_time)
        std::string message;
        message += hostname_;
        message += ',';
        message += std::to_string(end_count);
        message += ',';
        message += std::to_string(execution_time);
        send(sockfd, message.c_str(), message.size(), 0); // 送信

        // ソケットクローズ
        close(sockfd); 
    }

    // RW Manager　のリセット
    RW_manager_.reset();

    std::cout << "reset ok" << std::endl;
}