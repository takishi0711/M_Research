#pragma once

#include <string>
#include <unordered_map>
#include <deque>
#include <thread>
#include <mutex>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalkerGenerator {

public :

    // スロースタート
    void slowstart(int32_t node_ID, int32_t RWer_ID);
    
    // RG に RWer を登録
    void register_RWer(int32_t node_ID, int32_t RWer_ID);

    // RQ からの輻輳直接通知
    void congestion_notification_from_RQ(int32_t node_ID);

    // RWer の終了判定を flag に
    void set_RWer_flag(int32_t RWer_ID, bool flag);

    // 正常に終了した RWer に対する処理
    void fin_RWer_proc(int32_t node_ID, int32_t RWer_ID);

    // 終了した RWer の数 (開始頂点毎) を入手
    int get_end_count_of_RWer(int32_t node_ID);

    // 連続実行用のリセット関数
    void reset();

private : 
    std::unordered_map<int32_t, int> end_count_of_RWer_per_node; // 終了した RWer の数 (開始頂点毎)
    std::unordered_map<int32_t, bool> congestion_flag_per_node; // 直接通知による輻輳判定 (開始頂点毎)
    std::unordered_map<int32_t, std::deque<int32_t>> buffer_of_RWer_ID_per_node; // 生成した RWer の ID を格納するリングバッファ (開始頂点毎)
    std::unordered_map<int32_t, std::chrono::system_clock::time_point> start_time_per_RWer_ID; // RWer 毎の開始時刻
    std::unordered_map<int32_t, int> T_per_node; // RWer の推定帰還時間 T (micros) (開始頂点毎)
    std::unordered_map<int32_t, int> ave_RTT_per_node; // RWer の平均帰還時間 (micros) (開始頂点毎)
    std::unordered_map<int32_t, int> Deviation_per_node; // RWer の帰還時間の偏差 (micros) (開始頂点毎)
    std::unordered_map<int32_t, bool> end_flag_per_RWer; // 終了判定 (RWer 毎)
    std::unordered_map<int32_t, int> count_x_per_node; // sleep を決定するためのカウント (開始頂点毎)
    std::mutex mtx_end_count_of_RWer_per_node;
    std::mutex mtx_congestion_flag_per_node;
    std::mutex mtx_T_per_node;
    std::mutex mtx_end_flag_per_RWer;
    std::mutex mtx_start_time_per_RWer_ID;

    // パラメタ
    int initial_sleep_time = 10000; // sleepの初期値(ns)
    int X = 5; // T < t < X * T を探索
    int initial_count_x = 3; // count_x の初期値
    int max_size_of_RWer_ID_buffer = 50; // buffer_of_RWer_ID の最大サイズ

    double wa = 0.125; // 平均帰還時間の計算のためのパラメタ (RWer の平均帰還時間 = (1-wa) * RWer の平均帰還時間 + wa * RWer の帰還時間)
    double wb = 0.25; // 偏差の計算のためのパラメタ (Deviation = (1 - wb) * Deviation + wb * |RWer の帰還時間 - RWer の平均帰還時間|)

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void RandomWalkerGenerator::slowstart(int32_t node_ID, int32_t RWer_ID) {
    int cnt_end;
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_end_count_of_RWer_per_node);
        cnt_end = end_count_of_RWer_per_node[node_ID];
    }
    int sleep_time = 0;
    bool RWer_loss = false;

    if (cnt_end < 3) { // 最初の 3 回分の RWer が終了してない場合
        sleep_time = initial_sleep_time;
        count_x_per_node[node_ID] = initial_count_x;
    } else {
        // 起点ノードからの輻輳直接通知チェック (直接通知による RWer ロス検知)
        {
            std::unique_lock<std::mutex> uniq_lk(mtx_congestion_flag_per_node);
            RWer_loss = congestion_flag_per_node[node_ID];
            congestion_flag_per_node[node_ID] = false;
        }

        // リングバッファを前からなめていき, RWer ロスを探す (タイムアウトによる RWer ロス検知)
        int T;
        {
            std::unique_lock<std::mutex> uniq_lk(mtx_T_per_node);
            T = T_per_node[node_ID];
        }
        for (int i = 0; i < buffer_of_RWer_ID_per_node[node_ID].size(); i++) {
            // t : RWer 生成から経った時間
            int t;
            {
                std::unique_lock<std::mutex> uniq_lk(mtx_start_time_per_RWer_ID);
                t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time_per_RWer_ID[RWer_ID]).count();
            }

            // T < t < T * X の時にパケロスしているかどうかを見る
            if (t < T) {
                continue;
            } else if (t < X * T) {
                bool end_flag;
                {
                    std::unique_lock<std::mutex> uniq_lk(mtx_end_flag_per_RWer);
                    end_flag = end_flag_per_RWer[RWer_ID];
                }
                if (!end_flag) {
                    RWer_loss = true;
                    break;
                }
            } else {
                break;
            }
        }

        // RWer ロスの有無によってカウントを操作
        if (RWer_loss) { // RWer ロスあり
            count_x_per_node[node_ID] = initial_count_x;
        } else { // RWer ロスなし
            if (count_x_per_node[node_ID] > 0) {
                count_x_per_node[node_ID]--;
            }
        }

        // カウント値によって sleep 値を決定
        if (count_x_per_node[node_ID] == 0) {
            sleep_time = 0;
        } else {
            sleep_time = 1;
        }
    }

    // sleep を実行
    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
}

inline void RandomWalkerGenerator::register_RWer(int32_t node_ID, int32_t RWer_ID) {
    // RWer をリングバッファに追加
    buffer_of_RWer_ID_per_node[node_ID].push_front(RWer_ID);
    if (buffer_of_RWer_ID_per_node[node_ID].size() > max_size_of_RWer_ID_buffer) { // 最大サイズをオーバーしたら後ろのものをpop
        buffer_of_RWer_ID_per_node[node_ID].pop_back();
    }

    // RWer の終了判定を false に (初期化)
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_end_flag_per_RWer);
        end_flag_per_RWer[RWer_ID] = false;
    }

    // RWer の生成時刻を記録
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_start_time_per_RWer_ID);
        start_time_per_RWer_ID[RWer_ID] = std::chrono::system_clock::now(); 
    }
}

inline void RandomWalkerGenerator::congestion_notification_from_RQ(int32_t node_ID) {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_congestion_flag_per_node);
        congestion_flag_per_node[node_ID] = true;
    }
}

inline void RandomWalkerGenerator::set_RWer_flag(int32_t RWer_ID, bool flag) {
    // RWer の終了判定を flag に
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_end_flag_per_RWer);
        end_flag_per_RWer[RWer_ID] = flag;
    }
}

inline void RandomWalkerGenerator::fin_RWer_proc(int32_t node_ID, int32_t RWer_ID) {
    // RWer の生存時間計測
    int t;
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_start_time_per_RWer_ID);
        t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time_per_RWer_ID[RWer_ID]).count();
    }

    // T_per_node (RWer の推定帰還時間) の更新 (移動平均)
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_T_per_node);

        ave_RTT_per_node[node_ID] = ave_RTT_per_node[node_ID] + wa * (t - ave_RTT_per_node[node_ID]);

        Deviation_per_node[node_ID] = (1.0-wb) * Deviation_per_node[node_ID] + wb * std::abs(t - ave_RTT_per_node[node_ID]);

        T_per_node[node_ID] = ave_RTT_per_node[node_ID] + 4 * Deviation_per_node[node_ID];
    }

    // RWer の終了判定を true に
    set_RWer_flag(RWer_ID, true);

    // 開始頂点毎の終了した RWer の数を加算
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_end_count_of_RWer_per_node);
        end_count_of_RWer_per_node[node_ID]++;
    }
}

inline int RandomWalkerGenerator::get_end_count_of_RWer(int32_t node_ID) {
    {
        std::unique_lock<std::mutex> uniq_lk(mtx_end_count_of_RWer_per_node);
        return end_count_of_RWer_per_node[node_ID];
    }
}

inline void RandomWalkerGenerator::reset() {
    end_count_of_RWer_per_node.clear();
    std::cout << 1 << std::endl;
    congestion_flag_per_node.clear();
    std::cout << 2 << std::endl;
    buffer_of_RWer_ID_per_node.clear();
    std::cout << 3 << std::endl;
    start_time_per_RWer_ID.clear();
    std::cout << 4 << std::endl;
    T_per_node.clear();
    std::cout << 5 << std::endl;
    ave_RTT_per_node.clear();
    std::cout << 6 << std::endl;
    Deviation_per_node.clear();
    std::cout << 7 << std::endl;
    end_flag_per_RWer.clear();
    std::cout << 8 << std::endl;
    count_x_per_node.clear();
    std::cout << 9 << std::endl;

    // デバッグ
    std::cout << "RG ok" << std::endl;
}