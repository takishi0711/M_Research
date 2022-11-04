#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class MessageQueue {

public :

    // message_queue_ にメッセージを Push 
    void push(char* message);

    // message_queue_ からメッセージを取り出す
    char* pop();

private :
    std::queue<char*> message_queue_;
    std::mutex mtx_message_queue_;
    std::condition_variable cv_message_queue_;  

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void MessageQueue::push(char* message) {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

        bool queue_empty = message_queue_.empty();

        message_queue_.push(message);

        if (queue_empty) { // 空キューでなくなった通知
            cv_message_queue_.notify_all();
        }
    }
}

inline char* MessageQueue::pop() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

        // RWer_Queue が空じゃなくなるまで待機
        cv_message_queue_.wait(uniq_lk, [&]{ return !message_queue_.empty(); });

        char* message = message_queue_.front();

        message_queue_.pop();

        return message;
    }

}