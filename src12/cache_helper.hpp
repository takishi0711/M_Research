#pragma once

#include <tuple>
#include <list>
#include <unordered_map>
#include <mutex>
#include <iostream>

#include "param.hpp"


template<class T> size_t HashCombine(const size_t seed,const T &v){
    return seed^(std::hash<T>()(v)+0x9e3779b9+(seed<<6)+(seed>>2));
}
/* tuple用 */
template<int N> struct HashTupleCore{
    template<class Tuple> size_t operator()(const Tuple &keyval) const noexcept{
        size_t s=HashTupleCore<N-1>()(keyval);
        return HashCombine(s,std::get<N-1>(keyval));
    }
};
template <> struct HashTupleCore<0>{
    template<class Tuple> size_t operator()(const Tuple &keyval) const noexcept{ return 0; }
};
template<class... Args> struct std::hash<std::tuple<Args...>>{
    size_t operator()(const tuple<Args...> &keyval) const noexcept {
        return HashTupleCore<tuple_size<tuple<Args...>>::value>()(keyval);
    }
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class LRU {

public:

    void setCapacity(const uint32_t& capacity);

    // 隣接リスト情報内の index 存在確認
    // 存在したら next node ID を返す
    // 存在しなかったら INF を返す
    uint32_t getNextNode(const uint32_t& node_ID, const uint32_t& index_num);

    // index を登録
    // capacity をオーバーしていれば一番最後に使われた index を削除
    void putIndex(const uint32_t& node_ID_u, const uint32_t& index_num, const uint32_t& node_ID_v);

    // debug 用
    void printList();
    uint32_t getSize();

private:

    std::list<std::tuple<uint32_t, uint32_t, uint32_t>> que_; // (node_u, index, node_v)
    std::unordered_map<std::tuple<uint32_t, uint32_t>, std::list<std::tuple<uint32_t, uint32_t, uint32_t>>::iterator> iter_; // (<(node_u, index), iterator>) 
    uint32_t cap_ = MAX_CACHE_SIZE;

    std::mutex mtx_lru_;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void LRU::setCapacity(const uint32_t& capacity) {
    cap_ = capacity;
}

inline uint32_t LRU::getNextNode(const uint32_t& node_ID, const uint32_t& index_num) {
    {
        std::lock_guard<std::mutex> lock(mtx_lru_);

        std::tuple<uint32_t, uint32_t> k{node_ID, index_num};

        if (!iter_.contains(k)) {
            return INF;
        }

        auto it = iter_[k];
        auto [node_u, idx, node_v] = *it;

        que_.erase(it);
        que_.push_front({node_u, idx, node_v});
        iter_[k] = que_.begin();

        return node_v;
    }
}

inline void LRU::putIndex(const uint32_t& node_ID_u, const uint32_t& index_num, const uint32_t& node_ID_v) {
    {
        std::lock_guard<std::mutex> lock(mtx_lru_);

        std::tuple<uint32_t, uint32_t> k{node_ID_u, index_num};

        if (iter_.contains(k)) {
            que_.erase(iter_[k]);
        }

        que_.push_front({node_ID_u, index_num, node_ID_v});
        iter_[k] = que_.begin();
        if (que_.size() > cap_) {
            auto [node_u, idx, node_v] = *(--que_.end());
            iter_.erase({node_u, idx});
            que_.pop_back();
        }
    }
}

inline void LRU::printList() {
    for (auto it = que_.begin(); it != que_.end(); it++) {
        auto [node_u, index, node_v] = *it;
        std::cout << node_u << " " << index << " " << node_v << std::endl;
    }
}

inline uint32_t LRU::getSize() {
    return que_.size();
}