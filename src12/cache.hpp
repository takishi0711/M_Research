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
    uint32_t getDegree(const uint32_t& node_id);

    // キャッシュの次数存在確認
    bool hasDegree(const uint32_t& node_id);

    // 隣接リスト情報内の index 存在確認
    // 存在したら next node ID を返す
    // 存在しなかったら INF を返す
    uint32_t getNextNode(const uint32_t& node_id, const uint32_t& index_num);

    // ノードの持ち主の ホストIDを入手
    uint32_t getHostId(const uint32_t& node_id);

    // RWer の経路情報からグラフデータをキャッシュとして保存
    void addRWer(std::unique_ptr<RandomWalker>&& RWer_ptr, Graph& graph);

    // エッジをキャッシュに登録
    void addEdge(const std::vector<uint64_t>& path, const uint32_t& node_u_idx, const uint32_t& node_v_idx, Graph& graph);

    // ホストID 情報を登録
    void registerHostId(const uint32_t& node_id, const uint32_t& host_id);

    // 次数情報を登録
    void registerDegree(const uint32_t& node_id, const uint32_t& degree);

    // インデックス情報を登録
    void registerIndex(const uint32_t& node_id_u, const uint32_t& node_id_v, const uint32_t& index_num);

private :

    // キャッシュ情報
    std::unordered_map<uint32_t, uint32_t> degree_; // 他サーバが持ち主となるノードの次数
    std::unordered_map<uint32_t, uint32_t> host_id_; // 持ち主が他サーバの頂点に関する, 持ち主のホストID {ノード ID : ホストID (ノードの持ち主)}
    // std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> adjacency_list_; // 他サーバの隣接リスト {ノード ID : {index : ノード ID}}
    LRU adjacency_list_; // 他サーバの隣接リスト

    //　mutex
    std::shared_mutex mtx_cache_degree_; // 次数用
    std::shared_mutex mtx_cache_adj_; // 隣接リスト用
    std::shared_mutex mtx_cache_host_id_; // hostID 用

};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline uint32_t Cache::getDegree(const uint32_t& node_id) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_id]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache_degree_);

        return degree_.at(node_id);
    }
}

inline bool Cache::hasDegree(const uint32_t& node_id) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_cache_degree_);

        return degree_.contains(node_id);
    }
}

inline uint32_t Cache::getNextNode(const uint32_t& node_id, const uint32_t& index_num) {
    return adjacency_list_.getNextNode(node_id, index_num);
}

inline uint32_t Cache::getHostId(const uint32_t& node_id) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_id]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache_host_id_);

        return host_id_.at(node_id);
    }
}

inline void Cache::addRWer(std::unique_ptr<RandomWalker>&& RWer_ptr, Graph& graph) {
    // debug
    std::cout << "addRWer" << std::endl;

    uint16_t path_length = 0;
    std::vector<uint64_t> path; // path: (頂点, ホストID, 次数, indexuv, indexvu), (), (), ... 
    RWer_ptr->getPath(path_length, path);

    // debug
    // std::cout << "path_length: " << path_length << std::endl;
    // std::cout << "path size: " << path.size() << std::endl;

    for (int i = 0; i < path_length-1; i++) {
        // エッジをキャッシュに追加 (無向グラフ)
        addEdge(path, i*5, (i+1)*5, graph);

        // debug 
        std::cout << i << std::endl;
    }

    // debug 
    std::cout << "addRWer ok" << std::endl;
}

inline void Cache::addEdge(const std::vector<uint64_t>& path, const uint32_t& node_u_idx, const uint32_t& node_v_idx, Graph& graph) {
    // debug
    // std::cout << "AddEdge" << std::endl;

    uint32_t node_id_u = path[node_u_idx];
    uint32_t node_id_v = path[node_v_idx];
    uint32_t host_id_u = path[node_u_idx + 1];
    uint32_t host_id_v = path[node_v_idx + 1];
    uint32_t degree_u = path[node_u_idx + 2];
    uint32_t degree_v = path[node_v_idx + 2];
    uint32_t index_uv = path[node_v_idx + 3];
    uint32_t index_vu = path[node_v_idx + 4];

    if (!graph.hasVertex(node_id_u)) {
        registerHostId(node_id_u, host_id_u);
        if (degree_u != INF) registerDegree(node_id_u, degree_u);
        if (index_uv != INF) registerIndex(node_id_u, node_id_v, index_uv);
    }

    if (!graph.hasVertex(node_id_v)) {
        registerHostId(node_id_v, host_id_v);
        if (degree_v != INF) registerDegree(node_id_v, degree_v);
        if (index_vu != INF) registerIndex(node_id_v, node_id_u, index_vu);
    }
}

inline void Cache::registerHostId(const uint32_t& node_id, const uint32_t& host_id) {
    // debug
    // std::cout << "registerHostId" << std::endl;

    bool exist = false;

    { // 共有ロックで存在だけ確認
        std::shared_lock<std::shared_mutex> lock(mtx_cache_host_id_);

        if (host_id_.contains(node_id)) exist = true;
    }

    if (exist == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_cache_host_id_);

            host_id_[node_id] = host_id;
        }
    }
}

inline void Cache::registerDegree(const uint32_t& node_id, const uint32_t& degree) {
    // debug
    // std::cout << "registerDegree" << std::endl;

    bool exist = false;

    { // 共有ロックで存在だけ確認
        std::shared_lock<std::shared_mutex> lock(mtx_cache_degree_);

        if (degree_.contains(node_id)) exist = true;
    }

    if (exist == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_cache_degree_);

            degree_[node_id] = degree;
        }
    }
}

inline void Cache::registerIndex(const uint32_t& node_id_u, const uint32_t& node_id_v, const uint32_t& index_num) {
    // debug
    // std::cout << "registerIndex" << std::endl;

    adjacency_list_.putIndex(node_id_u, index_num, node_id_v);
}