#pragma once

#include <queue>

#include "random_walker_queue.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SendQueue {

public :

    // Send_Queue に RWer を Push 
    void Push(RandomWalker& RWer);

    // Send_Queue から RWer を取り出す
    RandomWalker Pop();

private :
    std::queue<RandomWalker> Send_Queue;
    std::mutex mtx_Send_Queue;
    std::condition_variable cv_Send_Queue;  

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void SendQueue::Push(RandomWalker& RWer) {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_Send_Queue);

        bool queue_empty = Send_Queue.empty();

        Send_Queue.push(RWer);

        if (queue_empty) { // 空キューでなくなった通知
            cv_Send_Queue.notify_all();
        }
    }
}

inline RandomWalker SendQueue::Pop() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_Send_Queue);

        // RWer_Queue が空じゃなくなるまで待機
        cv_Send_Queue.wait(uniq_lk, [&]{ return !Send_Queue.empty(); });

        RandomWalker RWer = Send_Queue.front();

        Send_Queue.pop();

        return RWer;
    }

}