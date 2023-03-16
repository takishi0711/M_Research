#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>

#include "random_walker.hpp"
#include "graph.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct MessageQueue {

public :

    // message_queue_ にメッセージを Push 
    void push(std::unique_ptr<T>&& message) {
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

    // message_queue_ からメッセージを取り出す
    std::unique_ptr<T> pop() {
        { // 排他制御
            std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(uniq_lk, [&]{ return !message_queue_.empty(); });

            // // デバッグ
            // std::cout << "queue size: " << message_queue_.size() << std::endl;

            std::unique_ptr<T> message = std::move(message_queue_.front());

            // // デバッグ
            // std::cout << "pop: " << message.get() << std::endl;

            message_queue_.pop();

            return message;
        }
    }

    // message_queue_ からメッセージをまとめて取り出す (RWer 用)
    void pop(std::unordered_map<uint32_t, MessageQueue>& message_buffer, Graph& graph) {
        { // 排他制御
            std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(uniq_lk, [&]{ return !message_queue_.empty(); });

            while (message_queue_.size()) {

                std::unique_ptr<RandomWalker> RWer = std::move(message_queue_.front());

                // 送信先 IP アドレス
                uint32_t ip = graph.getIP(RWer->getCurrentNode());

                message_buffer[ip].push(std::move(RWer));

                message_queue_.pop();
            }
        }        
    }

    // message_queue_ のサイズを入手
    uint32_t getSize() {
        { // 排他制御
            std::unique_lock<std::mutex> uniq_lk(mtx_message_queue_);

            return message_queue_.size();
        }        
    }

private :
    std::queue<std::unique_ptr<T>> message_queue_;
    std::mutex mtx_message_queue_;
    std::condition_variable cv_message_queue_;  

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////