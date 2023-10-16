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

    void push(std::unique_ptr<T>&& message) {
        { // 排他制御
            std::lock_guard<std::mutex> lk(mtx_message_queue_);

            bool queue_empty = message_queue_.empty();

            message_queue_.push(std::move(message));

            if (queue_empty) { // 空キューでなくなった通知
                cv_message_queue_.notify_all();
            }

        }
    }

    // message_queue_ からメッセージを取り出す
    std::unique_ptr<T> pop() {
        { // 排他制御
            std::unique_lock<std::mutex> lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(lk, [&]{ return !message_queue_.empty(); });

            // // デバッグ
            // std::cout << "queue size: " << message_queue_.size() << std::endl;

            std::unique_ptr<T> message = std::move(message_queue_.front());

            // // デバッグ
            // std::cout << "pop: " << message.get() << std::endl;

            message_queue_.pop();

            return message;
        }
    }

    // message_queue_ からメッセージをまとめて取り出す (RWer 用、送信先毎の送信キュー)
    // メッセージ長を返す
    uint32_t pop(char* message, const uint32_t& MESSAGE_MAX_LENGTH, const uint32_t& hostip, uint16_t& RWer_count) {
        { // 排他制御
            std::unique_lock<std::mutex> lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(lk, [&]{ return !message_queue_.empty(); });

            uint32_t now_length = 0;

            while (message_queue_.size()) {

                std::unique_ptr<RandomWalker> RWer_ptr = std::move(message_queue_.front());

                message_queue_.pop();

                // RWer データサイズ
                uint32_t RWer_data_length = RWer_ptr->getRWerSize();

                if (now_length + RWer_data_length >= MESSAGE_MAX_LENGTH) { // メッセージに収まりきらなくなったら終了
                    message_queue_.push(RWer);
                    return now_length;
                }

                // RWerの中身をメッセージに詰める
                // memcpy(message + now_length, &RWer, RWer_data_length);
                RWer_ptr->writeMessage(message + now_length);
                now_length += RWer_data_length;
            }

            return now_length;
        }
    }

    // message_queue_ のサイズを入手
    uint32_t getSize() {
        { // 排他制御
            std::lock_guard<std::mutex> lk(mtx_message_queue_);

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