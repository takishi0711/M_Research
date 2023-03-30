#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

#include "random_walker.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SendQueue {

public :

    // send_queue に RWer を Push 
    void push(const RandomWalker& RWer);

    // send_queue から RWer を取り出す
    RandomWalker pop();

    // send_queue_ のサイズを入手
    int32_t getSize();

private :
    std::queue<RandomWalker> send_queue_;
    std::mutex mtx_send_queue_;
    std::condition_variable cv_send_queue_;  

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void SendQueue::push(const RandomWalker& RWer) {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_send_queue_);

        bool queue_empty = send_queue_.empty();

        send_queue_.emplace(RWer);

        if (queue_empty) { // 空キューでなくなった通知
            cv_send_queue_.notify_all();
        }
    }
}

inline RandomWalker SendQueue::pop() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_send_queue_);

        // RWer_Queue が空じゃなくなるまで待機
        cv_send_queue_.wait(uniq_lk, [&]{ return !send_queue_.empty(); });

        RandomWalker RWer = send_queue_.front();

        send_queue_.pop();

        return RWer;
    }

}

inline int32_t SendQueue::getSize() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_send_queue_);

        return send_queue_.size();
    }
}