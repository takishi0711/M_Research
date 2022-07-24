#pragma once

#include <queue>

#include "random_walker_queue.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct SendFinQueue {

private :
    std::queue<RandomWalker> Send_Fin_Queue;
    std::mutex mtx_Send_Fin_Queue;
    std::condition_variable cv_Send_Fin_Queue; 

public :

    // send_fin_queue に RWer を Push 
    void Push(RandomWalker RWer);

    // Send_Fin_Queue から RWer を取り出す
    RandomWalker Pop(); 

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void SendFinQueue::Push(RandomWalker RWer) {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_Send_Fin_Queue);

        bool queue_empty = Send_Fin_Queue.empty();

        Send_Fin_Queue.push(RWer);

        if (queue_empty) { // 空キューでなくなった通知
            cv_Send_Fin_Queue.notify_all();
        }
    }
}

inline RandomWalker SendFinQueue::Pop() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_Send_Fin_Queue);

        // RWer_Queue が空じゃなくなるまで待機
        cv_Send_Fin_Queue.wait(uniq_lk, [&]{ return !Send_Fin_Queue.empty(); });

        RandomWalker RWer = Send_Fin_Queue.front();

        Send_Fin_Queue.pop();

        return RWer;
    }

}