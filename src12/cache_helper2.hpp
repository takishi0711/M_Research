#pragma once

#include <unordered_map>
#include <shared_mutex>

#include "param.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SimpleCache {

public:

    // void setCapacity(const uint32_t& capacity);

    // 隣接リスト情報内の index 存在確認
    // 存在したら next node ID を返す
    // 存在しなかったら INF を返す
    vertex_id_t getNextNode(const vertex_id_t& node_ID, const index_t& index_num);

    // index を登録
    // 
    void putIndex(const vertex_id_t& node_ID_u, const index_t& index_num, const vertex_id_t& node_ID_v);

    // // cache[node_ID][index_num] が存在するかどうか
    // bool isExist(const vertex_id_t& node_ID, const index_t& index_num);

    // debug 用
    // void printList();
    uint32_t getSize();

private:

    std::unordered_map<vertex_id_t, std::unordered_map<index_t, vertex_id_t>> cache_;
    uint32_t cache_size_ = 0;

    std::shared_mutex mtx_cache_;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline vertex_id_t SimpleCache::getNextNode(const vertex_id_t& node_ID, const index_t& index_num) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_cache_);

        if (!cache_.contains(node_ID)) return INF;
        if (!cache_[node_ID].contains(index_num)) return INF;

        return cache_[node_ID][index_num];
    }
}

inline void SimpleCache::putIndex(const vertex_id_t& node_ID_u, const index_t& index_num, const vertex_id_t& node_ID_v) {
    if (cache_size_ >= MAX_CACHE_SIZE) return;

    bool exist_edge = false;
    {
        std::shared_lock<std::shared_mutex> lock(mtx_cache_);
        if (cache_.contains(node_ID_u)) {
            if (cache_[node_ID_u].contains(index_num)) exist_edge = true;
        } 
    }

    if (!exist_edge) {
        {
            std::lock_guard<std::shared_mutex> lock(mtx_cache_);

            cache_[node_ID_u][index_num] = node_ID_v;
            cache_size_++;
            if (cache_size_ >= MAX_CACHE_SIZE) USE_CACHE = false;
        }
    }
}

inline uint32_t SimpleCache::getSize() {
    uint32_t cnt = 0;
    for (auto mp : cache_) {
        cnt += mp.second.size();
    }
    return cnt;
}