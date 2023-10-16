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

// ver_id_ (8bit): 
// バージョン: 4bit, メッセージID: 4bit
// メッセージ ID について, 0 -> 生存した RWer, 1 -> 終了した RWer, 2 -> 複数の RWer が入っているパケット, 3 -> 実験開始の合図, 4 -> 実験終了の合図
// 
// flag_ (8bit): 
// 一歩前で通信が発生したか: 1bit, あまり : 7bit
// 
// RWer_size_ (16bit):
// RWer 単体のメモリサイズ
//
// RWer_id_ (32bit):
//
// RWer_life_ (16bit):
// RWer の残り歩数
// 
// path_length_at_current_host_ (16bit):
// RWer の現在の同一ホスト内の経路長
//
// reserved_ (32bit):
// 予備
// 
// next_index_ (64bit):
// 通信が発生した時の次の遷移先 index
//
// path_ (64bit の可変長配列):
// 経路情報
// {HostID(48bit) + 同HostID内の経路長(15bit) + 通信が発生したか(1bit)}, {頂点(64bit), 次数(64bit), u->v の index(64bit), v->u の index(64bit), 頂点, 次数, ...}, {HostID(48bit) + 同HostID内の経路長(15bit) + 通信が発生したか(1bit)}, ...


struct RandomWalker {

public :

    // コンストラクタ
    RandomWalker();
    RandomWalker(const uint64_t& source_node, const uint64_t& node_degree, const uint32_t& RWer_id, const uint64_t& HostID, const uint32_t& RWer_life);
    RandomWalker(const char* message); // メッセージから RWer 復元

    // メッセージIDを入れる
    void inputMessageID(const uint8_t& id);

    // メッセージID取得
    uint8_t getMessageID();

    // RWer を終了させる
    void endRWer();

    // RWer が終了しているかどうか (true: 終了, false: 生存)
    bool isEnd();

    // 通信が発生したときに flag を入れる
    void inputSendFlag(bool flag); 

    // 一歩前で通信が発生したか (true: した, false: してない)
    bool isSended();

    // RWer のサイズを入手 (Byte 単位)
    uint32_t getRWerSize();

    // RWer_life を入力 (RWer の指定歩数, 寿命)
    void inputRWerLife(const uint16_t& life);

    // RWer_life を入手
    uint16_t getRWerLife();

    // RWer の経路長を入手
    uint8_t getPathLength();

    // RWer の ID を入手
    uint32_t getRWerID();

    // path_ の次の index の位置 (頂点を入れる場所) を入手
    uint32_t getNextIndexOfPath();

    // 現在のノードの path_ の index を入手
    uint32_t getCurrentIndexOfPath();

    // 現在のノードを入手
    uint64_t getCurrentNode();

    // 現在の Host index を入手
    uint64_t getCurrentHostIndex();

    // 現在のノードの HostID を入手
    uint64_t getCurrentNodeHostID();

    // 一歩前のノードの path_ の index を入手
    uint32_t getPrevIndexOfPath();

    // 一歩前のノードを入手
    uint64_t getPrevNode();

    // 起点サーバの IP アドレスを入手
    uint64_t getHostID();

    // 通信が発生した時の次の遷移先 index を入力
    void setIndex(const uint64_t& index_num);

    // 通信が発生した時の次の遷移先 index を返す
    uint64_t getNextIndex();

    // RWer の現在ノードを更新
    void updateRWer(const uint64_t& next_node, const uint64_t& host_id, const uint64_t& node_degree, const uint64_t& index_uv, const uint64_t& index_vu);

    // 一歩前の v -> u の index を登録
    void setPrevIndex(const uint64_t& index_num);

    // 現在の頂点の次数を入力
    void setCurrentDegree(const uint64_t& node_degree);

    // 通ってきたサーバ集合 (IPアドレス) を入手
    std::unordered_set<uint64_t> getHostGroup();

    // path_ のメモリ確保のためのサイズを現在の RWer_size と RWer_life から算出
    uint16_t getRequiredPathSize();

    // RWer のデータを書き込む
    void writeMessage(char* message);


private :

    uint8_t ver_id_ = 0; 
    uint8_t flag_ = 0;
    uint16_t RWer_size_ = 0;
    uint32_t RWer_id_ = 0;    
    uint16_t RWer_life_ = 0; 
    uint16_t path_length_at_current_host_ = 0; 
    uint32_t reserved_ = 0; 
    uint64_t next_index_ = 0;
    std::vector<uint64_t> path_;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline RandomWalker::RandomWalker() {
    inputMessageID(ALIVE);
}

inline RandomWalker::RandomWalker(const uint64_t& source_node, const uint64_t& node_degree, const uint32_t& RWer_id, const uint64_t& HostID, const uint32_t& RWer_life) {
    inputMessageID(ALIVE);
    RWer_size_ = 8 + 8 + 8;
    path_length_at_current_host_ = 1;
    RWer_id_ = RWer_id;
    RWer_life_ = RWer_life;
    path_.resize(getRequiredPathSize());
    path_[0] = (HostID<<16) + (1<<1) + 1; // HostID, 同HostID内の経路長入力 (終了した後最初のホストには送信するので, 送信フラグを入れておく)
    path_[1] = source_node;
    path_[2] = node_degree;
}

inline RandomWalker::RandomWalker(const char* message) {
    int idx = 0;
    ver_id_ = *(uint8_t*)(message[idx]); idx += 1;
    flag_ = *(uint8_t*)(message[idx]); idx += 1;
    RWer_size_ = *(uint16_t*)(message[idx]); idx += 2;
    RWer_id_ = *(uint32_t*)(message[idx]); idx += 4;
    RWer_life_ = *(uint16_t*)(message[idx]); idx += 2;
    path_length_at_current_host_ = *(uint16_t*)(message[idx]); idx += 2;
    reserved_ = *(uint32_t*)(message[idx]); idx += 4;
    next_index_ = *(uint64_t*)(message[idx]); idx += 8;

    path_.resize(getRequiredPathSize());
    int last_idx = getNextIndexOfPath() - 1;
    for (int i = 0; i <= last_idx; i++) {
        path_[i] = *(uint64_t*)(message[idx]); idx += 8;
    }
}

inline void RandomWalker::inputMessageID(const uint8_t& id) {
    ver_id_ |= id;
}

inline uint8_t RandomWalker::getMessageID() {
    return ver_id_ & MASK_MESSEGEID;
}

inline void RandomWalker::endRWer() {
    inputMessageID(DEAD);
}

inline bool RandomWalker::isEnd() {
    return getMessageID() == DEAD;
}

inline void RandomWalker::inputSendFlag(bool flag) {
    flag_ &= ~(1<<7);
    flag_ |= (flag<<7);
}

inline bool RandomWalker::isSended() {
    return (flag_>>7)&1;
}


inline uint32_t RandomWalker::getRWerSize() {
    return RWer_size_;
}

inline uint8_t RandomWalker::getPathLength() {
    return path_length_at_current_host_;
}

inline void RandomWalker::inputRWerLife(const uint16_t& life) {
    RWer_life_ = life;
}

inline uint16_t RandomWalker::getRWerLife() {
    return RWer_life_;
}

inline uint32_t RandomWalker::getRWerID() {
    return RWer_id_;
}

inline uint32_t RandomWalker::getNextIndexOfPath() {
    // RWer_size_ から逆算
    return (RWer_size_ - 8 - 8 - 8) / 8;
}

inline uint32_t RandomWalker::getCurrentIndexOfPath() {
    return getNextIndexOfPath() - 4;
}

inline uint64_t RandomWalker::getCurrentNode() {
    return path_[getCurrentIndexOfPath()];
}

inline uint64_t RandomWalker::getCurrentHostIndex() {
    return getCurrentIndexOfPath() - (4*(path_length_at_current_host_ - 1) + 1);
}

inline uint64_t RandomWalker::getCurrentNodeHostID() {
    return path_[getCurrentHostIndex()]>>16;
}

inline uint32_t RandomWalker::getPrevIndexOfPath() {
    uint32_t prev_index;
    if (path_length_at_current_host_ == 1) prev_index = getNextIndexOfPath() - (4 + 1 + 4);
    else prev_index = getNextIndexOfPath() - (4 + 1 + 4);

    if (prev_index < 0) {
        perror("getPrevNode");
        exit(1); // 異常終了
    }

    return prev_index;
}

inline uint64_t RandomWalker::getPrevNode() {
    return path_[getPrevIndexOfPath()];
}

inline uint64_t RandomWalker::getHostID() {
    return path_[0]>>16;
}

inline void RandomWalker::setIndex(const uint64_t& index_num) {
    next_index_ = index_num;
}

inline uint64_t RandomWalker::getNextIndex() {
    return next_index_;
}

inline void RandomWalker::updateRWer(const uint64_t& next_node, const uint64_t& host_id, const uint64_t& node_degree, const uint64_t& index_uv, const uint64_t& index_vu) {
    uint32_t start_index = getNextIndexOfPath();
    if (getCurrentNodeHostID() != host_id) {
        path_[start_index++] = (host_id<<16); RWer_size_ += 8; // HostID 入力
        path_length_at_current_host_ = 0;
    }
    if (isSended()) {
        path_[getCurrentHostIndex()] |= 1; // 送信フラグを立てる
        inputSendFlag(false);
    } 

    path_[start_index++] = next_node; RWer_size_ += 8;
    path_[start_index++] = node_degree; RWer_size_ += 8;
    path_[start_index++] = index_uv; RWer_size_ += 8;
    path_[start_index++] = index_vu; RWer_size_ += 8;
    
    if (path_length_at_current_host_ < MAX_PATH_LENGTH - 1) path_length_at_current_host_++;
    path_[getCurrentHostIndex()] += (1<<1);

    RWer_life_--;
    if (RWer_life_ == 0) endRWer();
}

inline void RandomWalker::setPrevIndex(const uint64_t& index_num) {
    uint32_t prev_index = getPrevIndexOfPath();
    if (prev_index < 0) {
        perror("setPrevIndex");
        exit(1); // 異常終了
    }

    path_[prev_index + 3] = index_num;
}

inline void RandomWalker::setCurrentDegree(const uint64_t& node_degree) {
    path_[getCurrentIndexOfPath() + 1] = node_degree;
}

inline std::unordered_set<uint64_t> RandomWalker::getHostGroup() {
    std::unordered_set<uint64_t> host_set;
    uint32_t host_index = 0;
    uint32_t last_index_ = getNextIndexOfPath();
    while (1) {
        if (host_index == last_index_) break;
        uint64_t host_id = path_[host_index]>>16;
        host_set.insert(host_id);
        uint16_t path_length_ = (path_[host_index] & ~((host_id)<<16))>>1;
        host_index += 4 * path_length_ + 1;
    }
    return host_set;
}

inline uint16_t RandomWalker::getRequiredPathSize() {
    return (RWer_size_ - 8 - 8 - 8)/8 + RWer_life_*5;
}

inline void RandomWalker::writeMessage(char* message) {
    int idx = 0;
    memcpy(message + idx, &ver_id_, sizeof(uint8_t)); idx += sizeof(uint8_t);
    memcpy(message + idx, &flag_, sizeof(uint8_t)); idx += sizeof(uint8_t);
    memcpy(message + idx, &RWer_size_, sizeof(uint16_t)); idx += sizeof(uint16_t);
    memcpy(message + idx, &RWer_id_, sizeof(uint32_t)); idx += sizeof(uint32_t);
    memcpy(message + idx, &RWer_life_, sizeof(uint16_t)); idx += sizeof(uint16_t);
    memcpy(message + idx, &path_length_at_current_host_, sizeof(uint16_t)); idx += sizeof(uint16_t);
    memcpy(message + idx, &reserved_, sizeof(uint32_t)); idx += sizeof(uint32_t);
    memcpy(message + idx, &next_index_, sizeof(uint64_t)); idx += sizeof(uint64_t);
    
    int i = 0;
    while (idx < RWer_size_) {
        memcpy(message + idx, &path_[i], sizeof(uint64_t)); idx += sizeof(uint64_t);
        i++;
    }
}