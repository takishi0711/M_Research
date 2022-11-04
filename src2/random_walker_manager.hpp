#pragma once

#include <chrono>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalkerManager {

public :
    // RWer の総数を入力し, 配列を new
    void init(const uint32_t RWer_all);

    // 配列の初期化
    void reset(const uint32_t RWer_all);

    // RWer 生成時間の記録
    void setStartTime(const uint32_t RWer_id);

    // RWer 終了時間の記録
    void setEndTime(const uint32_t RWer_id);

private :
    bool* end_flag_per_RWer_id_ = nullptr; // RWer_id に対する終了判定
    std::chrono::system_clock::time_point* start_time_per_RWer_id_ = nullptr; // RWer_id に対する開始時刻
    std::chrono::system_clock::time_point* end_time_per_RWer_id_ = nullptr; // RWer_id に対する終了時刻

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void RandomWalkerManager::init(const uint32_t RWer_all) {
    end_flag_per_RWer_id_ = new bool[RWer_all];
    start_time_per_RWer_id_ = new std::chrono::system_clock::time_point[RWer_all];
    end_time_per_RWer_id_ = new std::chrono::system_clock::time_point[RWer_all];
}

inline void RandomWalkerManager::reset(const uint32_t RWer_all) {
    for (int id = 0; id < RWer_all; id++) {
        end_flag_per_RWer_id_[id] = false;
    }
}

inline void RandomWalkerManager::setStartTime(const uint32_t RWer_id) {
    start_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();
}

inline void RandomWalkerManager::setEndTime(const uint32_t RWer_id) {
    end_flag_per_RWer_id_[RWer_id] = true;
    end_time_per_RWer_id_[RWer_id] = std::chrono::system_clock::now();
}