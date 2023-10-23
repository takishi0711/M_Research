#pragma once

#include <chrono>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <iostream>

#include "param.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalkerManager {

public :
    // RWer の総数を入力し, 配列を new
    void init(const uint32_t& RWer_all);

    // 配列の初期化
    void reset();

    // RWer 生成時間の記録
    void setStartTime(const uint32_t& RWer_id);

    // RWer 終了時間の記録
    void setEndTime(const uint32_t& RWer_id);

    // end_count_ の加算
    void addEndCount();

    // max_surviving_RWer_now の加算
    void addMaxSurvivngRWer();

    // RWer の歩数を入力
    void setRWerLife(const uint32_t& RWer_id, const uint16_t& life);

    // node_id を入力
    void setNodeId(const uint32_t& RWer_id, const uint64_t& node_id);

    // 実行する予定の RWer の総数を入手
    uint32_t getRWerAll();

    // RWer 終了数の入手
    uint32_t getEndcnt();

    // RWer がスタートしているかどうか
    bool isStart(const uint32_t& RWer_id);

    // RWer が終了しているかどうか
    bool isEnd(const uint32_t& RWer_id);

    // RWer の生存時間 (ms) を入手 (終了した RWer 限定)
    double getRWerLifeTime(const uint32_t& RWer_id);

    // RWer の現在の生存時間 (ms) を入手
    double getRWerLifeTimeNow(const uint32_t& RWer_id);

    // RWer の歩数を入手
    uint16_t getRWerLife(const uint32_t& RWer_id);

    // node_id を入手
    uint32_t getNodeId(const uint32_t& RWer_id);

    // 実行時間 (s) を入手 (一番遅い RWer の終了時間 - 一番早く生成された RWer の生成時間)
    double getExecutionTime();

    // 生存しているRWer数がMAX_SURVIVING_RWERを超えている限りロックし続ける
    void lockWhileOver();

    // surviving_RWer_の数を出力
    void printSurvivingRWerNum();

    // surviving_RWer_の数を取得
    uint32_t getSurvivingRWerNum();

    // MAX_SURVIVING_RWER を取得
    uint32_t getMaxSurvivingRwer();

    // max_surviving_RWer_now を取得
    uint32_t getMaxSurvivingRwerNow();

private :

    uint32_t RWer_all_num_ = 0; // RWer の総数
    bool* start_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    bool* end_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    std::chrono::system_clock::time_point* start_time_per_RWer_id_ = nullptr; // RWer_id に対する開始時刻
    std::chrono::system_clock::time_point* end_time_per_RWer_id_ = nullptr; // RWer_id に対する終了時刻
    uint16_t* RWer_life_per_RWer_id_ = nullptr; // RWer_id に対する設定歩数
    uint64_t* node_id_per_RWer_id_ = nullptr; // RWer_id に対する node_id
    uint32_t max_surviving_RWer_now = MAX_SURVIVING_RWER;

    uint32_t start_count_ = 0;
    uint32_t end_count_ = 0;
    std::mutex mtx_start_count_;
    std::mutex mtx_end_count_;
    std::condition_variable cv_start_count_;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void RandomWalkerManager::init(const uint32_t& RWer_all) {
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
    delete[] RWer_life_per_RWer_id_; RWer_life_per_RWer_id_ = nullptr;
    delete[] node_id_per_RWer_id_; node_id_per_RWer_id_ = nullptr;
}

inline void RandomWalkerManager::setStartTime(const uint32_t& RWer_id) {
    start_flag_per_RWer_id_[RWer_id] = true;
    start_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();
}

inline void RandomWalkerManager::setEndTime(const uint32_t& RWer_id) {
    // debug
    std::cout << "SetEndTime" << std::endl;

    if (end_flag_per_RWer_id_[RWer_id] == true) {
        return;
    }

    end_flag_per_RWer_id_[RWer_id] = true;
    end_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();

    addEndCount();
}

inline void RandomWalkerManager::addEndCount() {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_end_count_);
        end_count_++;
        if (start_count_ - end_count_ < max_surviving_RWer_now) cv_start_count_.notify_all();
    }
}

inline void RandomWalkerManager::addMaxSurvivngRWer() {
    max_surviving_RWer_now++;
}

inline void RandomWalkerManager::setRWerLife(const uint32_t& RWer_id, const uint16_t& life) {
    RWer_life_per_RWer_id_[RWer_id] = life;
}

inline void RandomWalkerManager::setNodeId(const uint32_t& RWer_id, const uint64_t& node_id) {
    node_id_per_RWer_id_[RWer_id] = node_id;
}

inline uint32_t RandomWalkerManager::getRWerAll() {
    return RWer_all_num_;
}

inline uint32_t RandomWalkerManager::getEndcnt() {
    return end_count_;
}

inline bool RandomWalkerManager::isStart(const uint32_t& RWer_id) {
    return start_flag_per_RWer_id_[RWer_id];
}

inline bool RandomWalkerManager::isEnd(const uint32_t& RWer_id) {
    return end_flag_per_RWer_id_[RWer_id];
}

inline double RandomWalkerManager::getRWerLifeTime(const uint32_t& RWer_id) {
    if (end_flag_per_RWer_id_[RWer_id] == false) {
        perror("getRWerLifeTime");
        exit(1);
    }

    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_per_RWer_id_[RWer_id] - start_time_per_RWer_id_[RWer_id]).count();
    return execution_time;
}

inline double RandomWalkerManager::getRWerLifeTimeNow(const uint32_t& RWer_id) {
    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time_per_RWer_id_[RWer_id]).count();
    return execution_time;
}

inline uint16_t RandomWalkerManager::getRWerLife(const uint32_t& RWer_id) {
    return RWer_life_per_RWer_id_[RWer_id];
}

inline uint32_t RandomWalkerManager::getNodeId(const uint32_t& RWer_id) {
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

inline void RandomWalkerManager::lockWhileOver() {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_start_count_);
        cv_start_count_.wait(uniq_lk, [&]{ return (start_count_ - end_count_) < max_surviving_RWer_now;});
        start_count_++;
    }
}

inline void RandomWalkerManager::printSurvivingRWerNum() {
    uint32_t surviving_RWer_ = start_count_ - end_count_;
    std::cout << surviving_RWer_ << std::endl;
}

inline uint32_t RandomWalkerManager::getSurvivingRWerNum() {
    uint32_t surviving_RWer_ = start_count_ - end_count_;
    return surviving_RWer_;
}

inline uint32_t RandomWalkerManager::getMaxSurvivingRwer() {
    return MAX_SURVIVING_RWER;
}
inline uint32_t RandomWalkerManager::getMaxSurvivingRwerNow() {
    return max_surviving_RWer_now;
}