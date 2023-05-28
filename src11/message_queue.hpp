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
    void push(T& message) {
        { // 排他制御
            std::unique_lock<std::mutex> lk(mtx_message_queue_);

            bool queue_empty = message_queue_.empty();

            // // デバッグ
            // std::cout << "push: " << message.get() << std::endl;

            message_queue_.push(message);

            if (queue_empty) { // 空キューでなくなった通知
                cv_message_queue_.notify_all();
            }

            // debug
            // std::cout << "push ok" << std::endl;
            // std::cout << std::endl;
            
        }
    }

    // message_queue_ からメッセージを取り出す
    T pop() {
        { // 排他制御
            std::unique_lock<std::mutex> lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(lk, [&]{ return !message_queue_.empty(); });

            // // デバッグ
            // std::cout << "queue size: " << message_queue_.size() << std::endl;

            T message = message_queue_.front();

            // // デバッグ
            // std::cout << "pop: " << message.get() << std::endl;

            message_queue_.pop();

            return message;
        }
    }

    // message_queue_ からメッセージをまとめて取り出す (RWer 用、ポート番号毎の送信キュー)
    void pop(std::unordered_map<uint32_t, MessageQueue>& message_buffer, Graph& graph) {
        { // 排他制御
            std::unique_lock<std::mutex> lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(lk, [&]{ return !message_queue_.empty(); });

            while (message_queue_.size()) {

                RandomWalker RWer = message_queue_.front();

                // 送信先 IP アドレス
                uint32_t ip;
                if (RWer.getEndFlag() == 1) { // 終了している場合
                    ip = RWer.getHostip();
                } else { // 終了していない場合
                    ip = graph.getIP(RWer.getCurrentNode());
                }
                
                message_buffer[ip].push(RWer);

                message_queue_.pop();
            }
        }        
    }

    // message_queue_ からメッセージをまとめて取り出す (RWer 用、送信先毎の送信キュー)
    // メッセージ長を返す
    uint32_t pop(char* message, const uint32_t& MESSAGE_MAX_LENGTH, const uint32_t& hostip) {
        { // 排他制御
            std::unique_lock<std::mutex> lk(mtx_message_queue_);

            // RWer_Queue が空じゃなくなるまで待機
            cv_message_queue_.wait(lk, [&]{ return !message_queue_.empty(); });

            uint32_t now_length = 0;

            while (message_queue_.size()) {

                RandomWalker RWer = message_queue_.front();

                message_queue_.pop();

                RWer.inputPrevIp(hostip);
                RWer.setSendFlag(1);

                // RWer データサイズ
                uint32_t RWer_data_length = RWer.getSize();

                if (now_length + RWer_data_length >= MESSAGE_MAX_LENGTH) { // メッセージに収まりきらなくなったら終了
                    message_queue_.push(RWer);
                    return now_length;
                }

                // RWerの中身をメッセージに詰める
                memcpy(message + now_length, &RWer, RWer_data_length);
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
    std::queue<T> message_queue_;
    std::mutex mtx_message_queue_;
    std::condition_variable cv_message_queue_;  

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////