#pragma once

#include <string>
#include <vector>
#include <sstream>

const uint32_t MAX_PATH_LENGTH = 100;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// RWer のサイズは 4 + 4 + 4 + 2 + 2 + length*4= 16 + length*4
struct RandomWalker {

public :

    // コンストラクタ
    RandomWalker(const uint32_t& source_node, const uint32_t& RWer_id, const uint32_t& hostip);

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
    uint16_t getEndFlag();

    // RWer の現在ノードを更新
    void updateRWer(const uint32_t& next_node);

    // RWer を終了させる
    void endRWer();

    // RWer のサイズ
    uint32_t getSize();

private :
    
    uint32_t RWer_id_; // RWer の ID
    uint32_t hostip_; // 起点サーバの IP アドレス
    uint32_t current_node_; // 現在のノード
    uint16_t path_length_; // RWer の経路長
    uint16_t end_flag_; // RWが終了しているか
    uint32_t path_[100]; // RWer の経路情報

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline RandomWalker::RandomWalker(const uint32_t& source_node, const uint32_t& RWer_id, const uint32_t& hostip) {
    RWer_id_ = RWer_id;
    hostip_ = hostip;
    current_node_ = source_node;
    path_length_ = 1;
    end_flag_ = 0;
    path_[0] = source_node;
}

inline uint32_t RandomWalker::getRWerId() {
    return RWer_id_;
}

inline uint32_t RandomWalker::getSourceNode() {
    return path_[0];
}

inline uint32_t RandomWalker::getHostip() {
    return hostip_;
}

inline uint32_t RandomWalker::getCurrentNode() {
    return current_node_;
}

inline uint16_t RandomWalker::getPathLength() {
    return path_length_;
}

inline void RandomWalker::getPath(uint32_t* path) {
    for (int i = 0; i < path_length_; i++) {
        path[i] = path_[i];
    }
}

inline uint16_t RandomWalker::getEndFlag() {
    return end_flag_;
}

inline void RandomWalker::updateRWer(const uint32_t& next_node) {
    current_node_ = next_node;
    path_[path_length_] = next_node;
    path_length_++;
}

inline void RandomWalker::endRWer() {
    end_flag_ = 1;
}

inline uint32_t RandomWalker::getSize() {
    return 16 + path_length_ * 4;
}