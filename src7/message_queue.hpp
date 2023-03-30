#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct MessageQueue {

public :

    // message_queue_ にメッセージを Push 
    void push(std::unique_ptr<char[]>&& message);

    // message_queue_ からメッセージを取り出す
    std::unique_ptr<char[]> pop();

    // message_queue_ のサイズを入手
    uint32_t getSize();

private :
    std::queue<std::unique_ptr<char[]>> message_queue_;
    std::mutex mtx_message_queue_;
    std::condition_variable cv_message_queue_;  

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void MessageQueue::push(std::unique_ptr<char[]>&& message) {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

        bool queue_empty = message_queue_.empty();

        // // デバッグ
        // std::cout << "push: " << message.get() << std::endl;

        message_queue_.push(std::move(message));

        if (queue_empty) { // 空キューでなくなった通知
            cv_message_queue_.notify_all();
        }
    }
}

inline std::unique_ptr<char[]> MessageQueue::pop() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

        // RWer_Queue が空じゃなくなるまで待機
        cv_message_queue_.wait(uniq_lk, [&]{ return !message_queue_.empty(); });

        // // デバッグ
        // std::cout << "queue size: " << message_queue_.size() << std::endl;

        std::unique_ptr<char[]> message = std::move(message_queue_.front());

        // // デバッグ
        // std::cout << "pop: " << message.get() << std::endl;

        message_queue_.pop();

        return message;
    }

}

inline uint32_t MessageQueue::getSize() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

        return message_queue_.size();
    }
}