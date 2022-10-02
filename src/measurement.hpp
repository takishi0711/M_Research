#pragma once

#include <chrono>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct Measurement {

public :
    // 実行開始時間の記録
    void setStart();

    // 実行終了時間の記録
    void setEnd();

    // 実行時間の取得 (秒)
    double getExecutionTime();

private :
    std::chrono::system_clock::time_point start_server; // 実行開始時間
    std::chrono::system_clock::time_point end_server; // 実行終了時間

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void Measurement::setStart() {
    start_server = std::chrono::system_clock::now();
}

inline void Measurement::setEnd() {
    end_server = std::chrono::system_clock::now();
}

inline double Measurement::getExecutionTime() {
    double execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_server - start_server).count();
    execution_time /= 1000; // 秒変換
    return execution_time;
}

