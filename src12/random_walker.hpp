#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <iostream>

#include "param.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// メッセージID : 2bit
// 終了しているかのフラグ (0: 生存, 1: 終了) : 1bit
// 一歩前で通信が発生したか (0: してない, 1: した) : 1bit
// 経路長 : 8bit
// RWerID : 4B
// 通信が発生した時の遷移先 index : 8B
// 経路情報 : 8B単位の配列と1B単位の配列 (同一サーバ内の経路長)
// HostID, {頂点, 次数, u->v の index, v->u の index, 頂点, 次数, ...}, HostID, ...
// 1 頂点目の index 部分は通信が発生した遷移をbitで管理するスペースに (8B + 8B = 16B)

// RWer のサイズは 1 (メッセージID, 終了しているかのフラグ, 一歩前で通信が発生したか) + 1 (経路長) + 4 (RWerID) + 8 (通信が発生した時の遷移先 index) + 1*50 + (8*4) * 50 (大体の経路情報) = 1500B くらい


struct RandomWalker {

public :

    // コンストラクタ
    RandomWalker();
    RandomWalker(const uint32_t& source_node, const uint32_t& node_degree, const uint32_t& RWer_id, const uint32_t& HostID);

    // メッセージIDを入れる
    void inputMessageID();

    // メッセージID取得
    uint8_t getMessageId();

    // RWer を終了させる
    void endRWer();

    // RWer が終了しているかどうか (true: 終了, false: 生存)
    bool isEnd();

    // 通信が発生したときに send_flag_ に bool を入れる
    void inputSendFlag(bool flag); 

    // 一歩前で通信が発生したか (true: した, false: してない)
    bool isSended();

    // RWer の経路長を入手
    uint8_t getPathLength();

    // RWer の ID を入手
    uint32_t getRWerId();

    // サーバ内経路長の配列の末端 index
    uint32_t getLastIndexOfServerPathLength();

    // path_ の次の index の位置 (頂点を入れる場所) を入手
    uint32_t getNextIndexOfPath();

    // 現在のノードの path_ の index を入手
    uint32_t getCurrentIndexOfPath();

    // 現在のノードを入手
    uint64_t getCurrentNode();

    // 一歩前のノードの path_ の index を入手
    uint32_t getPrevIndexOfPath();

    // 一歩前のノードを入手
    uint64_t getPrevNode();

    // 起点サーバの IP アドレスを入手
    uint64_t getHostID();

    // RWer のサイズを取得 (Byte 単位)
    uint32_t getSize();

    // path_ 配列の長さを返す
    uint32_t getPathArraySize();

    // 通信が発生した時の次の遷移先 index を入力
    void setIndex(const uint64_t& index_num);

    // 通信が発生した時の次の遷移先 index を返す
    uint64_t getNextIndex();

    // RWer の現在ノードを更新
    void updateRWer(const uint64_t& next_node, const uint64_t& node_ip, const uint64_t& node_degree, const uint64_t& index_uv, const uint64_t& index_vu);

    // 一歩前の v -> u の index を登録
    void setPrevIndex(const uint64_t& index_num);

    // 現在の頂点の次数を入力
    void setCurrentDegree(const uint64_t& node_degree);

    // 通ってきたサーバ集合 (IPアドレス) を入手
    std::unordered_set<uint32_t> getServerGroup();


private :

    uint8_t id_flag_; // メッセージID: 2bit, 終了しているかのフラグ: 1bit, 一歩前で通信が発生したか: 1bit (2 | 1 | 1 | 4あまり)
    uint8_t path_length_; // RWer の経路長
    uint32_t RWer_id_; // RWer の ID    
    uint64_t next_index_ = 0; // 通信が発生した時の遷移先 index
    uint8_t server_path_length_[MAX_PATH_LENGTH];
    uint64_t path_[MAX_PATH_LENGTH*5]; // RWer の経路情報

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline RandomWalker::RandomWalker() {
    inputMessageID();
}

inline RandomWalker::RandomWalker(const uint32_t& source_node, const uint32_t& node_degree, const uint32_t& RWer_id, const uint32_t& HostID) {
    inputMessageID();
    RWer_id_ = RWer_id;
    path_length_ = 1;
    server_path_length_[0] = 1;
    path_[0] = HostID;
    path_[1] = source_node;
    path_[2] = node_degree;
}

inline void RandomWalker::inputMessageID() {
    id_flag_ |= (1<<7);
}

inline uint8_t RandomWalker::getMessageId() {
    return id_flag_ >> 6;
}

inline void RandomWalker::endRWer() {
    id_flag_ |= (1<<5);
}

inline bool RandomWalker::isEnd() {
    return (id_flag_>>5)&1;
}

inline void RandomWalker::inputSendFlag(bool flag) {
    id_flag_ |= (1<<4);
}

inline bool RandomWalker::isSended() {
    return (id_flag_>>4)&1;
}

inline uint8_t RandomWalker::getPathLength() {
    return path_length_;
}

inline uint32_t RandomWalker::getRWerId() {
    return RWer_id_;
}

inline uint32_t RandomWalker::getLastIndexOfServerPathLength() {
    uint32_t return_idx = 0;
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        if (server_path_length_[i] == 0) break;
        return_idx = i;
    }
    return return_idx;
}

inline uint32_t RandomWalker::getNextIndexOfPath() {
    uint8_t now_length = path_length_;
    uint32_t return_idx = 0;
    uint32_t server_idx = 0;
    while (now_length) {
        if (now_length - server_path_length_[server_idx] <= 0) {
            return_idx++; // HostID
            return_idx += 4*now_length; 
            break;
        }

        return_idx++; // HostID
        return_idx += 4*server_path_length_[server_idx];
        now_length -= server_path_length_[server_idx];
        server_idx++;
    }
    return return_idx;
}

inline uint32_t RandomWalker::getCurrentIndexOfPath() {
    return getNextIndexOfPath() - 4;
}

inline uint64_t RandomWalker::getCurrentNode() {
    return path_[getCurrentIndexOfPath()];
}

inline uint32_t RandomWalker::getPrevIndexOfPath() {
    if (path_length_ < 2) {
        perror("getPrevNode");
        exit(1); // 異常終了
    }

    uint32_t last_server_path_length = server_path_length_[getLastIndexOfServerPathLength()];

    if (last_server_path_length == 1) { // current_node が今のサーバで一歩目のとき, 先頭に HostID が入っている
        return getNextIndexOfPath() - (4 + 1 + 4);
    } else {
        return getNextIndexOfPath() - (4 + 4);
    }
}

inline uint64_t RandomWalker::getPrevNode() {
    return path_[getPrevIndexOfPath()];
}

inline uint64_t RandomWalker::getHostID() {
    return path_[0];
}

inline uint32_t RandomWalker::getSize() {
    return 1 + 1 + 4 + 8 + 1*MAX_PATH_LENGTH + 8*getPathArraySize();
}

inline uint32_t RandomWalker::getPathArraySize() {
    uint32_t path_array_size = 0;
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        if (server_path_length_[i] == 0) break;
        path_array_size += 1; // HostID
        path_array_size += 4*server_path_length_[i];
    }
    return path_array_size;
}

inline void RandomWalker::setIndex(const uint64_t& index_num) {
    next_index_ = index_num;
}

inline uint64_t RandomWalker::getNextIndex() {
    return next_index_;
}

inline void RandomWalker::updateRWer(const uint64_t& next_node, const uint64_t& node_ip, const uint64_t& node_degree, const uint64_t& index_uv, const uint64_t& index_vu) {
    uint32_t start_index = getNextIndexOfPath();
    if (isSended()) {
        path_[4] |= (1<<path_length_);
        path_[start_index++] = node_ip;
        server_path_length_[getLastIndexOfServerPathLength() + 1]++;
    } else {
        server_path_length_[getLastIndexOfServerPathLength()]++;
    }
    path_[start_index++] = next_node;
    path_[start_index++] = node_degree;
    path_[start_index++] = index_uv;
    path_[start_index++] = index_vu;
    if (path_length_ < MAX_PATH_LENGTH - 1) path_length_++;
}

inline void RandomWalker::setPrevIndex(const uint64_t& index_num) {
    if (path_length_ < 2) {
        perror("setPrevIndex");
        exit(1); // 異常終了
    }

    path_[getPrevIndexOfPath() + 3] = index_num;
}

inline void RandomWalker::setCurrentDegree(const uint64_t& node_degree) {
    path_[getCurrentIndexOfPath() + 1] = node_degree;
}

inline std::unordered_set<uint32_t> RandomWalker::getServerGroup() {
    std::unordered_set<uint32_t> ip_set;
    uint32_t now_path_array_index = 0;
    uint32_t now_path_index = 0;
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        if (server_path_length_[i] == 0) break;

        uint64_t server_id = path_[now_path_array_index++];
        for (int j = 0; j < server_path_length_[i]; j++) {
            if ((path_[4]>>now_path_index++)&1) ip_set.insert(server_id);
        }
    }
    return ip_set;
}