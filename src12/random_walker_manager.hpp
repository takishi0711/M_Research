#pragma once

#include <chrono>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <atomic>

#include "param.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalkerManager {

public :
    // RWer の総数を入力し, 配列を new
    void init(const walker_id_t& RWer_all);

    // 配列の初期化
    void reset();

    // RWer 生成時間の記録
    void setStartTime(const walker_id_t& RWer_id);

    // RWer 終了時間の記録
    void setEndTime(const walker_id_t& RWer_id);

    // RWer の歩数を入力
    void setRWerLife(const walker_id_t& RWer_id, const uint16_t& life);

    // node_id を入力
    void setNodeId(const walker_id_t& RWer_id, const vertex_id_t& node_id);

    // 実行する予定の RWer の総数を入手
    walker_id_t getRWerAll();

    // RWer 終了数の入手
    walker_id_t getEndcnt();

    // RWer がスタートしているかどうか
    bool isStart(const walker_id_t& RWer_id);

    // RWer が終了しているかどうか
    bool isEnd(const walker_id_t& RWer_id);

    // RWer の生存時間 (ms) を入手 (終了した RWer 限定)
    double getRWerLifeTime(const walker_id_t& RWer_id);

    // RWer の現在の生存時間 (ms) を入手
    double getRWerLifeTimeNow(const walker_id_t& RWer_id);

    // RWer の歩数を入手
    uint16_t getRWerLife(const walker_id_t& RWer_id);

    // node_id を入手
    vertex_id_t getNodeId(const walker_id_t& RWer_id);

    // 実行時間 (s) を入手 (一番遅い RWer の終了時間 - 一番早く生成された RWer の生成時間)
    double getExecutionTime();

private :

    walker_id_t RWer_all_num_ = 0; // RWer の総数
    bool* start_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    bool* end_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    std::chrono::system_clock::time_point* start_time_per_RWer_id_ = nullptr; // RWer_id に対する開始時刻
    std::chrono::system_clock::time_point* end_time_per_RWer_id_ = nullptr; // RWer_id に対する終了時刻
    uint16_t* RWer_life_per_RWer_id_ = nullptr; // RWer_id に対する設定歩数
    vertex_id_t* node_id_per_RWer_id_ = nullptr; // RWer_id に対する node_id
    // std::atomic_bool* start_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    // std::atomic_bool* end_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    // std::atomic<std::chrono::system_clock::time_point>* start_time_per_RWer_id_ = nullptr; // RWer_id に対する開始時刻
    // std::atomic<std::chrono::system_clock::time_point>* end_time_per_RWer_id_ = nullptr; // RWer_id に対する終了時刻
    // std::atomic_uint16_t* RWer_life_per_RWer_id_ = nullptr; // RWer_id に対する設定歩数
    // std::atomic_uint64_t* node_id_per_RWer_id_ = nullptr; // RWer_id に対する node_id

    walker_id_t start_count_ = 0;
    std::atomic<walker_id_t> end_count_ = 0;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void RandomWalkerManager::init(const walker_id_t& RWer_all) {
    // RWer_all_num_ = RWer_all;
    // start_flag_per_RWer_id_ = new std::atomic_bool[RWer_all];
    // end_flag_per_RWer_id_ = new std::atomic_bool[RWer_all];
    // start_time_per_RWer_id_ = (std::atomic<std::chrono::system_clock::time_point>*)malloc(RWer_all * sizeof(std::atomic<std::chrono::system_clock::time_point>));
    // end_time_per_RWer_id_ = (std::atomic<std::chrono::system_clock::time_point>*)malloc(RWer_all * sizeof(std::atomic<std::chrono::system_clock::time_point>));
    // RWer_life_per_RWer_id_ = new std::atomic_uint16_t[RWer_all];
    // node_id_per_RWer_id_ = new std::atomic_uint64_t[RWer_all];

    RWer_all_num_ = RWer_all;
    start_flag_per_RWer_id_ = new bool[RWer_all];
    end_flag_per_RWer_id_ = new bool[RWer_all];
    start_time_per_RWer_id_ = new std::chrono::system_clock::time_point[RWer_all];
    end_time_per_RWer_id_ = new std::chrono::system_clock::time_point[RWer_all];
    RWer_life_per_RWer_id_ = new uint16_t[RWer_all];
    node_id_per_RWer_id_ = new uint64_t[RWer_all];
}

inline void RandomWalkerManager::reset() {
    delete[] start_flag_per_RWer_id_; start_flag_per_RWer_id_ = nullptr;
    delete[] end_flag_per_RWer_id_; end_flag_per_RWer_id_ = nullptr;
    delete[] start_time_per_RWer_id_; start_time_per_RWer_id_ = nullptr;
    delete[] end_time_per_RWer_id_; end_time_per_RWer_id_ = nullptr;
    // free(start_time_per_RWer_id_); start_time_per_RWer_id_ = nullptr;
    // free(end_time_per_RWer_id_); end_time_per_RWer_id_ = nullptr;
    delete[] RWer_life_per_RWer_id_; RWer_life_per_RWer_id_ = nullptr;
    delete[] node_id_per_RWer_id_; node_id_per_RWer_id_ = nullptr;
}

inline void RandomWalkerManager::setStartTime(const walker_id_t& RWer_id) {
    start_flag_per_RWer_id_[RWer_id] = true;
    start_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();
}

inline void RandomWalkerManager::setEndTime(const walker_id_t& RWer_id) {
    // debug
    // std::cout << "SetEndTime" << std::endl;

    if (end_flag_per_RWer_id_[RWer_id] == true) {
        return;
    }

    end_flag_per_RWer_id_[RWer_id] = true;
    end_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();

    // addEndCount();
    end_count_++;
}

inline void RandomWalkerManager::setRWerLife(const walker_id_t& RWer_id, const uint16_t& life) {
    RWer_life_per_RWer_id_[RWer_id] = life;
}

inline void RandomWalkerManager::setNodeId(const walker_id_t& RWer_id, const uint64_t& node_id) {
    node_id_per_RWer_id_[RWer_id] = node_id;
}

inline walker_id_t RandomWalkerManager::getRWerAll() {
    return RWer_all_num_;
}

inline walker_id_t RandomWalkerManager::getEndcnt() {
    return end_count_;

    // uint32_t end_count = 0;
    // for (int32_t id = 0; id < RWer_all_num_; id++) {
    //     if (end_flag_per_RWer_id_[id]) {
    //         end_count++;
    //     }
    // }
    // return end_count;
}

inline bool RandomWalkerManager::isStart(const walker_id_t& RWer_id) {
    return start_flag_per_RWer_id_[RWer_id];
}

inline bool RandomWalkerManager::isEnd(const walker_id_t& RWer_id) {
    return end_flag_per_RWer_id_[RWer_id];
}

inline double RandomWalkerManager::getRWerLifeTime(const walker_id_t& RWer_id) {
    if (end_flag_per_RWer_id_[RWer_id] == false) {
        perror("getRWerLifeTime");
        exit(1);
    }

    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_per_RWer_id_[RWer_id] - start_time_per_RWer_id_[RWer_id]).count();
    return execution_time;
}

inline double RandomWalkerManager::getRWerLifeTimeNow(const walker_id_t& RWer_id) {
    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time_per_RWer_id_[RWer_id]).count();
    return execution_time;
}

inline uint16_t RandomWalkerManager::getRWerLife(const walker_id_t& RWer_id) {
    return RWer_life_per_RWer_id_[RWer_id];
}

inline vertex_id_t RandomWalkerManager::getNodeId(const walker_id_t& RWer_id) {
    return node_id_per_RWer_id_[RWer_id];
}

inline double RandomWalkerManager::getExecutionTime() {
    std::chrono::system_clock::time_point min_start_time = start_time_per_RWer_id_[0];
    std::chrono::system_clock::time_point max_end_time = start_time_per_RWer_id_[0];
    for (int32_t id = 0; id < RWer_all_num_; id++) {
        min_start_time = std::min(min_start_time, start_time_per_RWer_id_[id]);
        if (end_flag_per_RWer_id_[id]) { // 終了しているもののみ
            max_end_time = std::max(max_end_time, end_time_per_RWer_id_[id]);
        }
    }
    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(max_end_time - min_start_time).count();
    execution_time /= 1000;
    return execution_time;
}

