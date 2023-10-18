#pragma once

#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include <vector>

#include "graph.hpp"
#include "random_walker.hpp"
#include "param.hpp"
#include "cache_helper.hpp"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class Cache {

public :

    // ノードに対するキャッシュの次数情報を入手
    uint32_t getDegree(const uint32_t& node_ID);

    // キャッシュの次数存在確認
    bool hasDegree(const uint32_t& node_ID);

    // 隣接リスト情報内の index 存在確認
    // 存在したら next node ID を返す
    // 存在しなかったら INF を返す
    uint32_t getNextNode(const uint32_t& node_ID, const uint32_t& index_num);

    // ノードの持ち主の IP アドレスを入手
    uint32_t getIP(const uint32_t& node_ID);

    // RWer の経路情報からグラフデータをキャッシュとして保存
    void addRWer(std::unique_ptr<RandomWalker>&& RWer_ptr, Graph& graph);

    // エッジをキャッシュに登録
    void addEdge(const std::vector<uint64_t>& path, const uint32_t& node_u_idx, const uint32_t& node_v_idx, Graph& graph);

    // ip アドレス情報を登録
    void registerIP(const uint32_t& node_ID, const uint32_t& ip);

    // 次数情報を登録
    void registerDegree(const uint32_t& node_ID, const uint32_t& degree);

    // インデックス情報を登録
    void registerIndex(const uint32_t& node_ID_u, const uint32_t& node_ID_v, const uint32_t& index_num);

private :

    // キャッシュ情報
    std::unordered_map<uint32_t, uint32_t> degree_; // 他サーバが持ち主となるノードの次数
    std::unordered_map<uint32_t, uint32_t> IP_; // 持ち主が他サーバの頂点に関する, 持ち主の IP アドレス {ノード ID : IP アドレス (ノードの持ち主)}
    // std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> adjacency_list_; // 他サーバの隣接リスト {ノード ID : {index : ノード ID}}
    LRU adjacency_list_; // 他サーバの隣接リスト

    //　mutex
    std::shared_mutex mtx_cache_degree_; // 次数用
    std::shared_mutex mtx_cache_adj_; // 隣接リスト用
    std::shared_mutex mtx_cache_IP_; // IP アドレス用

};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline uint32_t Cache::getDegree(const uint32_t& node_ID) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache_degree_);

        return degree_.at(node_ID);
    }
}

inline bool Cache::hasDegree(const uint32_t& node_ID) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_cache_degree_);

        return degree_.contains(node_ID);
    }
}

inline uint32_t Cache::getNextNode(const uint32_t& node_ID, const uint32_t& index_num) {
    return adjacency_list_.getNextNode(node_ID, index_num);
}

inline uint32_t Cache::getIP(const uint32_t& node_ID) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache_IP_);

        return IP_.at(node_ID);
    }
}

inline void Cache::addRWer(std::unique_ptr<RandomWalker>&& RWer_ptr, Graph& graph) {
    uint16_t path_length;
    std::vector<uint64_t> path; // path: (頂点, ホストID, 次数, indexuv, indexvu), (), (), ... 
    RWer_ptr->getPath(path_length, path);


    for (int i = 0; i < path_length-1; i++) {
        // エッジをキャッシュに追加 (無向グラフ)
        addEdge(path, i*5, (i+1)*5, graph);
    }
}

inline void Cache::addEdge(const std::vector<uint64_t>& path, const uint32_t& node_u_idx, const uint32_t& node_v_idx, Graph& graph) {
    uint32_t node_ID_u = path[node_u_idx];
    uint32_t node_ID_v = path[node_v_idx];
    uint32_t hostip_u = path[node_u_idx + 1];
    uint32_t hostip_v = path[node_v_idx + 1];
    uint32_t degree_u = path[node_u_idx + 2];
    uint32_t degree_v = path[node_v_idx + 2];
    uint32_t index_uv = path[node_v_idx + 3];
    uint32_t index_vu = path[node_v_idx + 4];

    registerIP(node_ID_u, hostip_u);
    registerIP(node_ID_v, hostip_v);
    registerDegree(node_ID_u, degree_u);
    registerDegree(node_ID_v, degree_v);

    if (!graph.hasVertex(node_ID_u) && index_uv != INF) {
        registerIndex(node_ID_u, node_ID_v, index_uv);
    }

    if (!graph.hasVertex(node_ID_v) && index_vu != INF) {
        registerIndex(node_ID_v, node_ID_u, index_vu);
    }
}

inline void Cache::registerIP(const uint32_t& node_ID, const uint32_t& ip) {
    bool exist = false;

    { // 共有ロックで存在だけ確認
        std::shared_lock<std::shared_mutex> lock(mtx_cache_IP_);

        if (IP_.contains(node_ID)) exist = true;
    }

    if (exist == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_cache_IP_);

            IP_[node_ID] = ip;
        }
    }
}

inline void Cache::registerDegree(const uint32_t& node_ID, const uint32_t& degree) {
    bool exist = false;

    { // 共有ロックで存在だけ確認
        std::shared_lock<std::shared_mutex> lock(mtx_cache_degree_);

        if (degree_.contains(node_ID)) exist = true;
    }

    if (exist == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_cache_degree_);

            degree_[node_ID] = degree;
        }
    }
}

inline void Cache::registerIndex(const uint32_t& node_ID_u, const uint32_t& node_ID_v, const uint32_t& index_num) {
    adjacency_list_.putIndex(node_ID_u, index_num, node_ID_v);
}