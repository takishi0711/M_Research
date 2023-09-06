#pragma once

#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>

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
    void addRWer(RandomWalker& RWer, Graph& graph);

    // エッジをキャッシュに登録
    void addEdge(uint32_t* node_u, uint32_t* node_v, Graph& graph);

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

inline void Cache::addRWer(RandomWalker& RWer, Graph& graph) {
    uint32_t path_length = RWer.getPathLength();
    uint32_t path[path_length*5];
    RWer.getPath(path);

    for (int i = 0; i < path_length-1; i++) {
        // エッジをキャッシュに追加 (無向グラフ)
        addEdge(&(path[i*5]), &(path[(i+1)*5]), graph);
    }
}

inline void Cache::addEdge(uint32_t* node_u, uint32_t* node_v, Graph& graph) {
    uint32_t node_ID_u = node_u[0];
    uint32_t node_ID_v = node_v[0];
    uint32_t hostip_u = node_u[1];
    uint32_t hostip_v = node_v[1];
    uint32_t degree_u = node_u[2];
    uint32_t degree_v = node_v[2];
    uint32_t index_uv = node_v[3];
    uint32_t index_vu = node_v[4];

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