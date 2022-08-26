#pragma once

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
#include <chrono>
#include <iostream>
#include <vector>
#include <sstream>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct StartManager {

private : 

    std::string myname; // StartManager のホスト名
    std::string myip; // StartManager の IP アドレス
    std::vector<std::string> worker_ip = {"10.58.60.3", "10.58.60.5", "10.58.60.6", "10.58.60.7", "10.58.60.8"}; // 実験で使う通信先 IP アドレス

public :

    // コンストラクタ
    StartManager();

    // 実験開始の合図
    void send_start();

    // IPv4 サーバソケットを作成
    int server_socket();

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline StartManager::StartManager() {
    // 自分のホスト名
    char hostname_c[128]; // ホスト名
    gethostname(hostname_c, sizeof(hostname_c)); // ホスト名を取得
    myname = hostname_c; // char* から string へ

    // 自分の IP アドレスを取得
    const char* ipname;
    if (myname[0] == 'a') ipname = "em1"; // abilene
    else if (myname[0] == 'e') ipname = "enp2s0"; // espresso
    else ipname = "vlan151"; // giji-4

    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    ifr.ifr_addr.sa_family = AF_INET; // IPv4のIPアドレスを取得したい
    strncpy(ifr.ifr_name, ipname, IFNAMSIZ-1); // em1のIPアドレスを取得したい
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    myip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr); // char*からstringへ
}

inline void StartManager::send_start() {
    // ソケットの生成
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { // エラー処理
        perror("socket");
        exit(1); // 異常終了
    }    

    std::chrono::system_clock::time_point start, end;

    // 計測開始
    start = std::chrono::system_clock::now(); 

    for (int i = 0; i < 5; i++) {
        // アドレスの生成
        struct sockaddr_in addr; // 接続先の情報用の構造体(ipv4)
        memset(&addr, 0, sizeof(struct sockaddr_in)); // memsetで初期化
        addr.sin_family = AF_INET; // アドレスファミリ(ipv4)
        addr.sin_port = htons(10000); // ポート番号, htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
        addr.sin_addr.s_addr = inet_addr(worker_ip[i].c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳        

        // メッセージ生成
        char message_ID = '1'; // メッセージ ID (start は 1)
        std::string message;
        message += message_ID;
        { // IP アドレスを 4 byte 表現に
            std::stringstream sstream(myip);
            std::string word;
            while (std::getline(sstream, word, '.')) {
                message += char(std::stoi(word));
            }
        }

        // データ送信
        sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&addr, sizeof(addr)); // 送信
    } 

    // ソケットクローズ
    close(sockfd);  

    // 5 個のサーバから終了の合図が来たら実験終了
    int end_count = 0; // 終了の合図が来た回数
    sockfd = server_socket(); // サーバソケットを生成
    while (end_count < 5) {
        char buf[1024]; // 受信バッファ
        memset(buf, 0, sizeof(buf)); // 受信バッファ初期化
        recv(sockfd, buf, sizeof(buf), 0); // 受信
        std::string message = buf; // メッセージをchar*型からstring型に変換

        end_count++;
    }

    // 計測終了
    end = std::chrono::system_clock::now(); 

    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 秒変換
    execution_time /= 1000;

    printf("Execution time : %.5f [sec]\n", execution_time);

    // ソケットクローズ
    close(sockfd);  
}

inline int StartManager::server_socket() {
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
    addr.sin_addr.s_addr = inet_addr(myip.c_str()); // IPアドレス, inet_addr()関数はアドレスの翻訳

    // ソケット登録
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { // ソケット, アドレスポインタ, アドレスサイズ // エラー処理
        perror("bind");
        exit(1); // 異常終了
    }

    return sockfd;
}