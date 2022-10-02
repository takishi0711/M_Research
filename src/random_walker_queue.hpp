#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>

#include "random_walker.hpp"
#include "random_walker_generator.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalkerQueue {

public :

    // RWer_Queue に RWer を Push 
    void Push(RandomWalker& RWer, RandomWalkerGenerator& RG, const std::string& hostip);

    // RED
    bool Random_Early_Detection(int queue_size);

    // RWer_Queue から RWer を取り出す
    RandomWalker Pop(); 

    // 連続実行用のリセット関数
    void reset();

private : 
    std::queue<RandomWalker> RWer_Queue;
    std::mutex mtx_RWer_Queue;
    std::condition_variable cv_RWer_Queue; 
    int drop_count = 0;

    // キューの種類のフラグ
    // 1 : RED
    // 2 : Tail Drop
    // 3 : ドロップなし (ため続ける)
    int queue_type = 1;

    // RED 関連
    double wq = 1;
    int minth = 10;
    int maxth = 100;
    double maxp = 0.1;
    int avg = 0;
    int count = -1;
    double c1 = maxp / (maxth - minth);
    double c2 = maxp * minth / (maxth - minth);

    // Tail Drop 関連
    int max_Tail = 100;

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成
    std::uniform_real_distribution<double>  rand_double{0, 1.0}; // 0~1のランダムな値
    
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void RandomWalkerQueue::Push(RandomWalker& RWer, RandomWalkerGenerator& RG, const std::string& hostip) {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_RWer_Queue);

        bool queue_empty = RWer_Queue.empty();

        if (queue_type == 1) { // RED(Random Early Detection)
            
            if (Random_Early_Detection(RWer_Queue.size())) { // Push
                // キューに Push
                RWer_Queue.push(RWer);

                if (queue_empty) { // 空キューでなくなった通知
                    cv_RWer_Queue.notify_all();
                }

            } else { // 破棄
                if (RWer.get_hostip() == hostip) { // もし起点サーバがここだったら起点ノードに輻輳を通知
                    // 輻輳通知
                    RG.congestion_notification_from_RQ(RWer.get_source_node());
                    // RWer の終了判定を true に
                    RG.set_RWer_flag(RWer.get_RWer_ID(), true);
                }
                drop_count++;
            }

        } else if (queue_type == 2) { // Tail Drop
            
            if (RWer_Queue.size() < max_Tail) { // Push
                // キューに Push
                RWer_Queue.push(RWer);

                if (queue_empty) { // 空キューでなくなった通知
                    cv_RWer_Queue.notify_all();
                }

            } else { // 破棄
                if (RWer.get_hostip() == hostip) { // もし起点サーバがここだったら起点ノードに輻輳を通知
                    // 輻輳通知
                    RG.congestion_notification_from_RQ(RWer.get_source_node());
                    // RWer の終了判定を true に
                    RG.set_RWer_flag(RWer.get_RWer_ID(), true);
                }
                drop_count++;
            }

        } else if (queue_type == 3) { // ドロップなし

            RWer_Queue.push(RWer);

            if (queue_empty) { // 空キューでなくなった通知
                cv_RWer_Queue.notify_all();
            }
        }
    }
}

inline bool RandomWalkerQueue::Random_Early_Detection(int queue_size) {
    if (minth <= queue_size && queue_size < maxth) { // minth <= queue_size < maxth
        count++;
        double pb = c1 * queue_size - c2;
        double pa = pb / (1.0 - pb * count);

        // 確率 pa で破棄
        if (rand_double(mt) < pa) {
            count = 0;
            return false; // 破棄
        } else {
            return true;
        }

    } else if (maxth <= queue_size) { // maxth <= queue_size
        count = 0;
        return false; // 破棄   

    } else { // queue_size < minth
        count = -1;
        return true;

    }
}

inline RandomWalker RandomWalkerQueue::Pop() {
    { // 排他制御
        std::unique_lock<std::mutex> uniq_lk(mtx_RWer_Queue);

        // RWer_Queue が空じゃなくなるまで待機
        cv_RWer_Queue.wait(uniq_lk, [&]{ return !RWer_Queue.empty(); });

        RandomWalker RWer = RWer_Queue.front();

        RWer_Queue.pop();

        return RWer;
    }

}

inline void RandomWalkerQueue::reset() {
    std::cout << "drop count : " << drop_count << std::endl;
    drop_count = 0;
    avg = 0;
    count = -1;

    // デバッグ
    std::cout << "RQ ok" << std::endl;
}
