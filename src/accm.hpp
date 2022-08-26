#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <random>
#include <thread>

#include "graph.hpp"
#include "random_walk.hpp"
#include "random_walker.hpp"
#include "random_walker_generator.hpp"
#include "random_walker_queue.hpp"
#include "send_fin_queue.hpp"
#include "send_queue.hpp"
#include "message1.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct ACCM {

private :
    std::string hostname; // 自サーバ のホスト名
    std::string hostip; // 自サーバ の IP アドレス
    std::string startmanagerip; // StartManager の IP アドレス
    Graph graph; // グラフデータ
    RandomWalk RW; // Random Walk 実行関連
    RandomWalkerGenerator RG; // RG 関連
    RandomWalkerQueue RQ; // RQ 関連
    SendFinQueue send_fin_queue; // RWer の終了通知のためのキュー
    SendQueue send_queue; // 他サーバへ遷移する RWer を格納するキュー
    Message1 start_message; // メッセージ 1 (開始の合図) に関する情報

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成
    std::uniform_real_distribution<double>  rand_double{0, 1.0}; // 0~1のランダムな値

public : 

    // コンストラクタ
    ACCM(std::string dir_path);

    // thread を開始させる用の関数 (generate_RWer(), one_hop_RW(), send_RWer(), send_fin_RWer(), receive_RWer())
    void start();
    
    // RWer 生成用の関数
    void generate_RWer();

    // RQ から RWer を取ってきて一歩進める関数
    void one_hop_RW();

    // Send_Queue から RWer を取ってきて他サーバへ送信する関数
    void send_RWer();

    // Send_Fin_Queue から RWer を取ってきて他サーバへ送信する関数
    void send_fin_RWer();

    // IPv4 サーバソケットを作成
    int server_socket();

    // 他サーバから RWer を受信し, 処理する関数
    void receive_RWer(int sockfd);

    // 全実行が終了した時に StartManager にメッセージを送る関数
    void send_to_startmanager();
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



inline ACCM::ACCM(std::string dir_path) {
    // 自サーバ のホスト名
    char hostname_c[128]; // ホスト名
    gethostname(hostname_c, sizeof(hostname_c)); // ホスト名を取得
    this->hostname = hostname_c; // char* から string へ

    // 自サーバ の IP アドレス
    const char* ipname;
    if (hostname[0] == 'a') ipname = "em1"; // abilene
    else if (hostname[0] == 'e') ipname = "enp2s0"; // espresso
    else ipname = "vlan151"; // giji-4

    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    ifr.ifr_addr.sa_family = AF_INET; // IPv4 の IP アドレスを取得したい
    strncpy(ifr.ifr_name, ipname, IFNAMSIZ-1); // ipname の IP アドレスを取得したい
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    this->hostip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr); // char* から string へ

    // グラフファイル読み込み
    graph.init(dir_path, hostname, hostip);

    // 全てのスレッドを開始させる
    start();    
}


inline void ACCM::start() {
    // スレッドを開始させる
    std::thread thread_generate_RWer(&ACCM::generate_RWer, this);
    std::thread thread_one_hop_RW(&ACCM::one_hop_RW, this);
    std::thread thread_send_RWer(&ACCM::send_RWer, this);
    std::thread thread_send_fin_RWer(&ACCM::send_fin_RWer, this);
    std::thread thread_receive_RWer(&ACCM::receive_RWer, this);

    // プログラムを終了させないようにする
    thread_one_hop_RW.join();
}


inline void ACCM::generate_RWer() {
    std::cout << "RWer_Generator" << std::endl;
    while (1) {
        // 開始通知を受けるまでロック
        start_message.lock_while_false();

        std::int32_t RWer_ID = 0;

        int number_of_RW_execution = RW.get_number_of_RW_execution();

        // 全てのノードから指定回数 の RW が終了するまで RWer を生成
        for (std::int32_t node_ID : graph.get_my_vertices()) { // 全てのノードから
            
            while (1) { // 指定回数 の RW が終了するまで RWer を生成
                // 指定回数分終わってたら終了
                if (RG.get_end_count_of_RWer(node_ID) == number_of_RW_execution) break;

                // RWer を生成
                RandomWalker RWer(node_ID, RWer_ID, hostip);

                // スロースタート (スリープ)
                RG.slowstart(node_ID, RWer_ID);

                // RG に RWer を登録
                RG.register_RWer(node_ID, RWer_ID);

                // RWer_Queue に Push
                RQ.Push(RWer, RG, hostip);

                RWer_ID++;
            }

        }

        send_to_startmanager();
    }
}

inline void ACCM::one_hop_RW() {
    std::cout << "one_hop_RW" << std::endl;
    while (1) {
        // RQ から RWer を入手
        RandomWalker RWer = RQ.Pop();

        // 現在ノードの隣接ノード集合を入手
        std::vector<std::int32_t> adjacency_vertices = graph.get_adjacency_vertices(RWer.get_current_node());

        // 次数
        int degree = adjacency_vertices.size();

        // RW を一歩進める
        if (rand_double(mt) < RW.get_alpha() || degree == 0) { // 確率 α もしくは次数 0 なら終了　

            if (RWer.get_hostip() == hostip) { // もし終了した RWer の起点サーバが今いるサーバである場合, 終了した RWer をこの場で処理

                RG.fin_RWer_proc(RWer.get_source_node(), RWer.get_RWer_ID());

            } else { // そうでないなら Send_fin_Queue に RWer を push

                send_fin_queue.Push(RWer);

            }

        } else { // 確率 1-α でランダムな隣接ノードへ遷移

            // ランダムな隣接ノードへ遷移
            std::uniform_int_distribution<int> rand_int(0, degree-1);
            std::int32_t next_node = adjacency_vertices[rand_int(mt)];
            RWer.update_RWer(next_node);

            // 遷移先ノードの持ち主が自サーバか他サーバかで分類
            if (graph.get_IP(next_node) == hostip) { // 自サーバへの遷移

                // RWer_Queue に Push
                RQ.Push(RWer, RG, hostip);
 
            } else { // 他サーバへの遷移

                // Send_Queue に RWer を Push
                send_queue.Push(RWer);

            }
        }
    }
}

inline void ACCM::send_RWer() {
    std::cout << "send_RWer" << std::endl;

    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }    

    while (1) {
        // Send_Queue から RWer を入手
        RandomWalker RWer = send_queue.Pop();

        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(10000); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = inet_addr(graph.get_IP(RWer.get_current_node()).c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

        // メッセージ作成
        char message_ID = '2'; // メッセージ ID (send_RWer は 2)
        std::string message;
        message += message_ID;
        message += RWer.serialize();  

       // データ送信
       sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&addr, sizeof(addr)); 
    }
}

inline void ACCM::send_fin_RWer() {
    std::cout << "send_fin_RWer" << std::endl;

    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if  (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }    

    while (1) {
        // Send_Fin_Queue から RWer を入手
        RandomWalker RWer = send_fin_queue.Pop();

        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(10000); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = inet_addr(graph.get_IP(RWer.get_current_node()).c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

        // メッセージ作成
        char message_ID = '3'; // メッセージ ID (send_fin_RWer は 3)
        std::string message;
        message += message_ID;
        message += RWer.serialize();  

        // データ送信
        sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&addr, sizeof(addr)); 
    }
}

inline int ACCM::server_socket() {
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
    addr.sin_port = htons(10000); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
    addr.sin_addr.s_addr = inet_addr(hostip.c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

    // ソケット登録
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { // ソケット, アドレスポインタ, アドレスサイズ // エラー処理
        perror("bind");
        exit(1); // 異常終了
    }

    return sockfd;
}

inline void ACCM::receive_RWer(int sockfd) {
    std::cout << "receive_RWer" << std::endl;

    while (1) {
        // messageを受信
        char buf[1400]; // 受信バッファ
        memset(buf, 0, sizeof(buf)); // 受信バッファ初期化
        recv(sockfd, buf, sizeof(buf), 0); // 受信
        std::string message = buf; // メッセージをchar*型からstring型に変換
        if (message.size() < 1) continue;

        // message -> メッセージ ID + メッセージ本体
        char message_ID = message[0];

        // メッセージ ID に則した処理
        if (message_ID == '1') { // ユーザーからの実験開始の合図

            // StartManager の IP アドレス
            std::string ip;
            for (int i = 0; i < 4; i++) {
                ip += std::to_string(int(message[1 + i]));
                ip += '.';
            }
            ip.pop_back();
            startmanagerip = ip;

            // 実験開始のフラグを立てる
            start_message.write_ready_M1(true);

        } else if (message_ID == '2') { // 他のサーバから遷移してきた RWer 
            
            // RWer へデシリアライズ
            message = message.substr(1);
            RandomWalker RWer(message);

            // RWer_Queue に Push
            RQ.Push(RWer, RG, hostip);

        } else if (message_ID == '3') { // 他のサーバで終了した RWer

            // RWer へデシリアライズ
            message = message.substr(1);
            RandomWalker RWer(message);

            // 終了した RWer の処理
            RG.fin_RWer_proc(RWer.get_source_node(), RWer.get_RWer_ID());

        } else { // その他はありえない
            perror("wrong id");
            exit(1); // 異常終了
        }

    }
}

inline void ACCM::send_to_startmanager() {
    std::cout << "send to startmanager" << std::endl;
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
    addr.sin_port = htons(10000); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
    addr.sin_addr.s_addr = inet_addr(startmanagerip.c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

    // データ送信
    std::string message;
    message += hostname;
    message += " end!!";
    sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&addr, sizeof(addr)); // 送信

    // ソケットクローズ
    close(sockfd);  
}