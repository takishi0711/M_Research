#pragma once

#include <chrono>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <iostream>

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

    // RWer 終了数の入手
    int32_t getEndcnt();

    // 実行時間 (秒) を入手 (一番遅い RWer の終了時間 - 一番早く生成された RWer の生成時間)
    double getExecutionTime();

    // 生存しているRWer数がMAX_SURVIVING_RWERを超えている限りロックし続ける
    void lockWhileOver();

    // surviving_RWer_の数を出力
    void printSurvivingRWerNum();

private :
    int32_t RWer_all_num_ = 0; // RWer の総数
    bool* end_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    std::chrono::system_clock::time_point* start_time_per_RWer_id_ = nullptr; // RWer_id に対する開始時刻
    std::chrono::system_clock::time_point* end_time_per_RWer_id_ = nullptr; // RWer_id に対する終了時刻

    uint32_t surviving_RWer_ = 0;
    std::mutex mtx_surviving_RWer_;
    std::condition_variable cv_surviving_RWer_;

    // パラメタ
    const uint32_t MAX_SURVIVING_RWER = 1500;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void RandomWalkerManager::init(const uint32_t& RWer_all) {
    RWer_all_num_ = RWer_all;
    end_flag_per_RWer_id_ = new bool[RWer_all];
    for (int id = 0; id < RWer_all_num_; id++) {
        end_flag_per_RWer_id_[id] = false;
    }
    start_time_per_RWer_id_ = new std::chrono::system_clock::time_point[RWer_all];
    end_time_per_RWer_id_ = new std::chrono::system_clock::time_point[RWer_all];
}

inline void RandomWalkerManager::reset() {
    for (int id = 0; id < RWer_all_num_; id++) {
        end_flag_per_RWer_id_[id] = false;
    }
    delete[] end_flag_per_RWer_id_; end_flag_per_RWer_id_ = nullptr;
    delete[] start_time_per_RWer_id_; start_time_per_RWer_id_ = nullptr;
    delete[] end_time_per_RWer_id_; end_time_per_RWer_id_ = nullptr;
    surviving_RWer_ = 0;
}

inline void RandomWalkerManager::setStartTime(const uint32_t& RWer_id) {
    start_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();
}

inline void RandomWalkerManager::setEndTime(const uint32_t& RWer_id) {
    end_flag_per_RWer_id_[RWer_id] = true;
    end_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();

    {
        std::unique_lock<std::mutex> uniq_lk(mtx_surviving_RWer_);
        surviving_RWer_--;
        if (surviving_RWer_ < MAX_SURVIVING_RWER) cv_surviving_RWer_.notify_all();
    }
}

inline int32_t RandomWalkerManager::getEndcnt() {
    int32_t end_count = 0;
    for (int32_t id = 0; id < RWer_all_num_; id++) {
        if (end_flag_per_RWer_id_[id]) {
            end_count++;
        }
    }
    return end_count;
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
        std::unique_lock<std::mutex> uniq_lk(mtx_surviving_RWer_);
        cv_surviving_RWer_.wait(uniq_lk, [&]{ return surviving_RWer_ < MAX_SURVIVING_RWER;});
        surviving_RWer_++;
    }
}

inline void RandomWalkerManager::printSurvivingRWerNum() {
    std::cout << surviving_RWer_ << std::endl;
}