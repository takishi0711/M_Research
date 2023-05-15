#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>

const uint32_t MAX_PATH_LENGTH = 50;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// RWer のサイズは 1 + 1 + 2 + 4 + (4 + 4 + 4)*50 = 608B (8 + 12 * path_length_)
// メッセージID : 1B
// 終了しているかのフラグ : 1B
// 経路長 : 2B
// RWerID : 4B
// 経路情報 : 頂点, HostIP, 次数 (4B, 4B, 4B)
class RandomWalker {

public :

    // コンストラクタ
    RandomWalker();
    RandomWalker(const uint32_t& source_node, const uint32_t& node_degree, const uint32_t& RWer_id, const uint32_t& hostip);

    // RWer の ID を入手
    uint32_t getRWerId();

    // 起点ノードを入手
    uint32_t getSourceNode();

    // 起点サーバの IP アドレスを入手
    uint32_t getHostip();

    // 現在のノードを入手
    uint32_t getCurrentNode();

    // RWer の経路長を入手
    uint16_t getPathLength();

    // RWer の経路情報を入手
    void getPath(uint32_t* path);

    // RWer の end_flag を入手
    uint8_t getEndFlag();

    // RWer の現在ノードを更新
    void updateRWer(const uint32_t& next_node, const uint32_t& node_ip, const uint32_t& node_degree);

    // 現在の頂点の次数を入力
    void inputCurrentDegree(const uint32_t& node_degree);

    // RWer を終了させる
    void endRWer();

    // RWer のサイズ
    uint32_t getSize();

    // 通ってきたサーバ集合 (IPアドレス) を入手
    std::unordered_set<uint32_t> getServerGroup();


private :
    
    uint8_t message_id_ = 2; // RWer の識別
    uint8_t end_flag_; // RWが終了しているか
    uint16_t path_length_; // RWer の経路長
    uint32_t RWer_id_; // RWer の ID    
    uint32_t path_[MAX_PATH_LENGTH*3]; // RWer の経路情報

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

inline uint32_t RandomWalker::getSourceNode() {
    return path_[0];
}

inline uint32_t RandomWalker::getHostip() {
    return path_[1];
}

inline uint32_t RandomWalker::getCurrentNode() {
    return path_[(path_length_ - 1) * 3];
}

inline uint16_t RandomWalker::getPathLength() {
    return path_length_;
}

inline void RandomWalker::getPath(uint32_t* path) {
    for (int i = 0; i < path_length_ * 3; i++) {
        path[i] = path_[i];
    }
}

inline uint8_t RandomWalker::getEndFlag() {
    return end_flag_;
}

inline void RandomWalker::updateRWer(const uint32_t& next_node, const uint32_t& node_ip, const uint32_t& node_degree) {
    path_[path_length_*3] = next_node;
    path_[path_length_*3 + 1] = node_ip;
    path_[path_length_*3 + 2] = node_degree;
    if (path_length_ < MAX_PATH_LENGTH - 1) path_length_++;
}

inline void RandomWalker::inputCurrentDegree(const uint32_t& node_degree) {
    path_[(path_length_-1)*3 + 2] = node_degree;
}

inline void RandomWalker::endRWer() {
    end_flag_ = 1;
}

inline uint32_t RandomWalker::getSize() {
    return 8 + (path_length_+1) * 12; // 一歩先の分まで確保
}

inline std::unordered_set<uint32_t> RandomWalker::getServerGroup() {
    std::unordered_set<uint32_t> ip_set;
    for (int i = 0; i < path_length_; i++) {
        ip_set.insert(path_[i*3 + 1]);
    }
    return ip_set;
}
