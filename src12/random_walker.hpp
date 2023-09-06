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

// メッセージID : 1B
// 終了しているかのフラグ : 1B
// 一歩前で通信が発生したか : 1B
// 経路長 : 1B
// RWerID : 4B
// 通信が発生した時の遷移先 index : 4B
// 経路情報 : 
//  if 1 頂点目 : 頂点, HostIP, 次数, 経路上で通信が発生した遷移をbitで管理 (4B, 4B, 4B, 8B)
//  else : 頂点, HostIP, 次数, u->v の index, v->u の index (4B, 4B, 4B, 4B, 4B)

// RWer のサイズは 1 + 1 + 1 + 1 + 4 + 4 + (4 + 4 + 4 + 4 + 4)*50 = 1012B (送信経路上では 12 + 20*path_length_)

struct RandomWalker {

public : 

    // コンストラクタ
    RandomWalker();
    RandomWalker(const uint32_t& source_node, const uint32_t& node_degree, const uint32_t& RWer_id, const uint32_t& hostip);

    // RWer の ID を入手
    uint32_t getRWerId();

    // 現在のノードを入手
    uint32_t getCurrentNode();

    // RWer の経路長を入手
    uint8_t getPathLength();

    // 起点サーバの IP アドレスを入手
    uint32_t getHostip();

    // RWer の end_flag を入手
    uint8_t getEndFlag();

    // 一歩前で通信が発生したか
    bool getSendFlag();

    // RWer の経路情報を入手
    void getPath(uint32_t* path);

    // RWer のサイズ
    uint32_t getSize();

    // 通信が発生した時の次の遷移先 index を返す
    uint32_t getNextIndex();

    // RWer の現在ノードを更新
    void updateRWer(const uint32_t& next_node, const uint32_t& node_ip, const uint32_t& node_degree, const uint32_t& index_uv, const uint32_t& index_vu);

    // index の set
    void setIndex(const uint32_t& index_num);

    // 現在の頂点の次数を入力
    void setCurrentDegree(const uint32_t& node_degree);

    // send_flag の中身を設定
    void setSendFlag(const uint8_t& flag);

    // RWer を終了させる
    void endRWer();

    // 通ってきたサーバ集合 (IPアドレス) を入手
    std::unordered_set<uint32_t> getServerGroup();

private : 

    uint8_t message_id_ = 2; // RWer の識別
    uint8_t end_flag_; // RWが終了しているか
    uint8_t send_flag_ = 0; // 送信が発生したかどうか
    uint8_t path_length_; // RWer の経路長
    uint32_t RWer_id_; // RWer の ID    
    uint32_t next_index_ = 0; // 通信が発生した時の遷移先 index
    uint32_t path_[MAX_PATH_LENGTH*5]; // RWer の経路情報

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline RandomWalker::RandomWalker() {}

inline RandomWalker::RandomWalker(const uint32_t& source_node, const uint32_t& node_degree, const uint32_t& RWer_id, const uint32_t& hostip) {
    RWer_id_ = RWer_id;
    path_length_ = 1;
    end_flag_ = 0;
    path_[0] = source_node;
    path_[1] = hostip;
    path_[2] = node_degree; 
}

inline uint32_t RandomWalker::getRWerId() {
    return RWer_id_;
}

inline uint32_t RandomWalker::getCurrentNode() {
    return path_[(path_length_ - 1) * 5];
}

inline uint8_t RandomWalker::getPathLength() {
    return path_length_;
}

inline uint32_t RandomWalker::getHostip() {
    return path_[1];
}

inline uint8_t RandomWalker::getEndFlag() {
    return end_flag_;
}

inline void RandomWalker::setCurrentDegree(const uint32_t& node_degree) {
    path_[(path_length_-1)*5 + 2] = node_degree;
}

inline bool RandomWalker::getSendFlag() {
    if (send_flag_ == 0) return false;
    else return true;
}

inline uint32_t RandomWalker::getNextIndex() {
    return next_index_;
}

inline void RandomWalker::updateRWer(const uint32_t& next_node, const uint32_t& node_ip, const uint32_t& node_degree, const uint32_t& index_uv, const uint32_t& index_vu) {
    path_[path_length_*5] = next_node;
    path_[path_length_*5 + 1] = node_ip;
    path_[path_length_*5 + 2] = node_degree;
    path_[path_length_*5 + 3] = index_uv;
    path_[path_length_*5 + 4] = index_vu;
    if (path_length_ < MAX_PATH_LENGTH - 1) path_length_++;
}

inline void RandomWalker::setIndex(const uint32_t& index_num) {
    next_index_ = index_num;
}

inline void RandomWalker::endRWer() {
    end_flag_ = 1;
}

inline void RandomWalker::getPath(uint32_t* path) {
    for (int i = 0; i < path_length_ * 5; i++) {
        path[i] = path_[i];
    }
}

inline std::unordered_set<uint32_t> RandomWalker::getServerGroup() {
    std::unordered_set<uint32_t> ip_set;
    for (int i; i < path_length_; i++) {
        if (path_[4]>>i&1) {
            ip_set.insert(path_[i*5 + 1]);
        }
    }
    return ip_set;
}

inline void RandomWalker::setSendFlag(const uint8_t& flag) {
    send_flag_ = flag;
}

inline uint32_t RandomWalker::getSize() {
    return 12 + (path_length_+1) * 20; // 一歩先の分まで確保
}