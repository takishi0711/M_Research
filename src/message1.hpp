#pragma once

#include <mutex>
#include <condition_variable>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct Message1 {

private :
    bool ready_M1; // M1 用のフラグ (実験開始の合図)
    std::mutex mtx_M1; // M1 用の mutex
    std::condition_variable cv_M1; // M1 用の condition variable

public :

    // ready_M1 への書き込み
    void write_ready_M1(bool flag);

    // ready_M1 の読み込み
    bool read_ready_M1();

    // ready_M1 が true になるまで (開始の合図が来るまで) ロックする関数
    void lock_while_false();

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void Message1::write_ready_M1(bool flag) {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_M1);
        ready_M1 = flag;
        cv_M1.notify_all();
    }
}


inline bool Message1::read_ready_M1() {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_M1);
        return ready_M1;
    }
}


inline void Message1::lock_while_false() {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_M1);
        cv_M1.wait(uniq_lk, [&]{ return ready_M1;});
        ready_M1 = false;
    }
}