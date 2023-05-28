#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <arpa/inet.h>
#include <shared_mutex>
#include <utility>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class Graph {

public : 

    // グラフファイル読み込み
    void init(const std::string& dir_path, const std::string& hostname, const uint32_t& hostip);

    // 辺の追加
    void addEdge(const std::vector<std::string>& words, const uint32_t& hostip);

    // 自サーバが持ち主となるノードの数を入手
    uint32_t getMyVerticesNum();

    // 自サーバが持ち主となるノード集合を入手
    std::vector<uint32_t> getMyVertices();

    // node_ID に対する隣接ノード集合を入手
    std::vector<uint32_t> getAdjacencyVertices(const uint32_t& node_ID);

    // ノードの持ち主の IP アドレスを入手
    uint32_t getIP(const uint32_t& node_ID);

    // ノードの次数を入手
    uint32_t getDegree(const uint32_t& node_ID);

    // 現在頂点とインデックスを引数にして次の頂点を返す
    uint32_t getNextNode(const uint32_t& current_node, const uint32_t& next_index);

    // ノードの持ち主が自サーバであるか確認
    bool hasVertex(const uint32_t& node_ID);

    // キャッシュノードごとの存在確認＆新規登録
    // mutex, hostip, degree, 隣接リスト の新規登録
    void nodeCache(const uint32_t& node_ID, const uint32_t& hostip, const uint32_t& degree);

    // キャッシュの次数存在確認
    bool hasCacheDegree(const uint32_t& node_ID);

    // ノードに対するキャッシュの次数情報を入手
    uint32_t getCacheDegree(const uint32_t& node_ID);

    // ノードに対するキャッシュの隣接リストを入手
    std::vector<uint32_t> getCacheAdjacencyList(const uint32_t& node_ID);

    // ノードに対するキャッシュの隣接リストの現在次数を入手
    uint32_t getCacheAdjacencyListSize(const uint32_t& node_ID);

    // 現在頂点とインデックスを引数にして次の頂点を返す
    uint32_t getCacheNextNode(const uint32_t& current_node, const uint32_t& next_index);

    // ノードに対するキャッシュのIPアドレスを入手
    uint32_t getCacheIP(const uint32_t& node_ID);

    // エッジをキャッシュに追加
    void addEdgeToCache(const uint32_t& node_ID_u, const uint32_t& hostip_u, const uint32_t& degree_u, const uint32_t& node_ID_v, const uint32_t& hostip_v, const uint32_t& degree_v, const std::unordered_set<uint32_t>& ip_set);

    // 引数のサーバが引数のエッジをキャッシュとして持っているかどうか
    bool hasServerCacheEdge(const uint32_t& ip, const uint32_t& node_ID_u, const uint32_t& node_ID_v);

    // キャッシュリセット
    void reset();

    // 元グラフのエッジカウント (デバッグ用)
    uint32_t getMyEdgeCount();

    // cacheグラフのエッジカウント (デバッグ用)
    uint32_t getCacheEdgeCount();
    
private :

    std::unordered_set<uint32_t> my_vertices_; // 自サーバが持ち主となるノード集合
    std::vector<uint32_t> my_vertices_vector_; // 自サーバが持ち主となるノード集合 (配列)
    std::unordered_map<uint32_t, uint32_t> vertices_IP_; // 自サーバが保持しているノードの持ち主の IP アドレス {ノード ID : IP アドレス (ノードの持ち主)}
    std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency_list_; // 自サーバの隣接リスト {ノード ID : 隣接リスト}
    std::unordered_map<uint32_t, uint32_t> degree_; // 自サーバが持ち主となるノードの次数

    std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::unordered_set<uint32_t>>> servers_who_used_edge_; // エッジ (u, v) をキャッシュとして保存しているであろうサーバ集合 <u, <v, サーバ集合>>

    // キャッシュ情報
    std::unordered_map<uint32_t, uint32_t> vertices_IP_cache_; // 持ち主が他サーバの頂点に関する, 持ち主の IP アドレス {ノード ID : IP アドレス (ノードの持ち主)}
    std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency_list_cache_; // 他サーバの隣接リスト {ノード ID : 隣接リスト}
    std::unordered_map<uint32_t, uint32_t> degree_cache_; // 他サーバが持ち主となるノードの次数

    // キャッシュのmutex
    std::unordered_map<uint32_t, std::shared_mutex> mtx_node_cache_;
    std::shared_mutex mtx_cache;
    // std::mutex mtx_vertices_IP_cache_;
    // std::mutex mtx_adjacency_list_cache_;
    // std::mutex mtx_degree_cache_;

    // mutex のための mutex
    std::shared_mutex mtx_mtx_node_cache_;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void Graph::init(const std::string& dir_path, const std::string& hostname, const uint32_t& hostip) {
    std::string graph_file_path = dir_path + hostname + ".txt"; // グラフファイルのパス
    std::ifstream reading_file;
    reading_file.open(graph_file_path, std::ios::in);
    std::string reading_line_buffer;
    while (std::getline(reading_file, reading_line_buffer)) { // 1 行ずつ読み取り
        std::vector<std::string> words; // [ノード 1, ノード 2, ノード 2 の IP アドレス]
        std::stringstream sstream(reading_line_buffer);
        std::string word;
        while (std::getline(sstream, word, ' ')) { // 空白区切りで word を取り出す
            words.emplace_back(word);
        }
        Graph::addEdge(words, hostip); // グラフデータに情報追加
    }  
    for (uint32_t node : my_vertices_) {
        my_vertices_vector_.emplace_back(node);
    }
}

inline void Graph::addEdge(const std::vector<std::string>& words, const uint32_t& hostip) { // words : [ノード 1, ノード 2, ノード 2 の IP アドレス]
    uint32_t u = std::stoi(words[0]), v = std::stoi(words[1]);
    my_vertices_.insert(u); // ノード 1 の持ち主は自分
    vertices_IP_[u] = hostip; // ノード 1 の IP アドレスを代入
    vertices_IP_[v] = inet_addr(words[2].c_str()); // ノード 2 の IP アドレスを代入
    adjacency_list_[u].emplace_back(v); // ノード １ の隣接リストにノード 2 を追加
    degree_[u] = adjacency_list_[u].size(); // 次数を更新

    if (vertices_IP_[v] != hostip) { // キャッシュノードの mutex を登録
        mtx_node_cache_[v].lock();
        mtx_node_cache_[v].unlock();
    }

    servers_who_used_edge_[u][v].size(); // エッジ (u, v) を登録
}

inline uint32_t Graph::getMyVerticesNum() {
    return my_vertices_.size();
}

inline std::vector<uint32_t> Graph::getMyVertices() {
    return my_vertices_vector_;
}

inline std::vector<uint32_t> Graph::getAdjacencyVertices(const uint32_t& node_ID) {
    return adjacency_list_.at(node_ID);
}

inline uint32_t Graph::getIP(const uint32_t& node_ID) {
    return vertices_IP_.at(node_ID);
}

inline uint32_t Graph::getDegree(const uint32_t& node_ID) {
    return degree_.at(node_ID);
}

inline uint32_t Graph::getNextNode(const uint32_t& current_node, const uint32_t& next_index) {
    return adjacency_list_.at(current_node)[next_index];
}

inline bool Graph::hasVertex(const uint32_t& node_ID) {
    return my_vertices_.contains(node_ID);
}

// ここからキャッシュ

inline void Graph::nodeCache(const uint32_t& node_ID, const uint32_t& hostip, const uint32_t& degree) {
    // debug
    // std::cout << "nodeCache" << std::endl;

    // ノードごとの mutex の登録
    bool exist_mtx = false;

    { // 共有ロックで存在だけ確認 
        std::shared_lock<std::shared_mutex> lock(mtx_mtx_node_cache_);

        if (mtx_node_cache_.contains(node_ID)) exist_mtx = true;
    }

    if (exist_mtx == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_mtx_node_cache_);

            mtx_node_cache_[node_ID];
        }
    }

    // ノードごとの IP, degree, 隣接リストを登録
    bool exist_node_data = false;

    { // 共有ロックで存在だけ確認
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        if (vertices_IP_cache_.contains(node_ID)) exist_node_data = true;
    }

    if (exist_node_data == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_cache);

            if (!vertices_IP_cache_.contains(node_ID)) {
                // adjacency_list_cache_[node_ID].reserve(500);
                adjacency_list_cache_[node_ID].reserve(degree + 10);
                degree_cache_[node_ID] = degree;
                vertices_IP_cache_[node_ID] = hostip;
            }
        }
    }
}

inline bool Graph::hasCacheDegree(const uint32_t& node_ID) {
    {
        // // debug
        // if (!mtx_node_cache_.contains(node_ID)) {
        //     perror("not exist mtx of node_ID");
        //     exit(1); // 異常終了
        // }

        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        return degree_cache_.contains(node_ID);
    }
}

inline uint32_t Graph::getCacheDegree(const uint32_t& node_ID) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        return degree_cache_.at(node_ID);
    }
}

inline std::vector<uint32_t> Graph::getCacheAdjacencyList(const uint32_t& node_ID) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        return adjacency_list_cache_.at(node_ID);
    }
}

inline uint32_t Graph::getCacheAdjacencyListSize(const uint32_t& node_ID) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        return adjacency_list_cache_.at(node_ID).size();
    }
}

inline uint32_t Graph::getCacheNextNode(const uint32_t& current_node, const uint32_t& next_index) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        return adjacency_list_cache_.at(current_node)[next_index];
    }
}

inline uint32_t Graph::getCacheIP(const uint32_t& node_ID) {
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        return vertices_IP_cache_.at(node_ID);
    }
}

inline void Graph::addEdgeToCache(const uint32_t& node_ID_u, const uint32_t& hostip_u, const uint32_t& degree_u, const uint32_t& node_ID_v, const uint32_t& hostip_v, const uint32_t& degree_v, const std::unordered_set<uint32_t>& ip_set) {
    // debug
    // std::cout << "addEdgeToCache: " << node_ID_u << " " << node_ID_v << std::endl;

    // u が自分のサーバのものだったらキャッシュに登録する必要はない
    // 自サーバのエッジをキャッシュとして登録するであろうサーバ集合を登録
    if (hasVertex(node_ID_u)) {

        for (uint32_t ip : ip_set) {
            servers_who_used_edge_[node_ID_u][node_ID_v].insert(ip);
        }
        
        return;
    }

    // 頂点 u, v についてキャッシュに登録されてなかったら登録
    nodeCache(node_ID_u, hostip_u, degree_u);
    nodeCache(node_ID_v, hostip_v, degree_v);

    // 追加エッジがすでにあるかどうか確認
    bool exist_edge = false;
    uint32_t vec_size = 0;
    {
        // std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID_u]);
        std::shared_lock<std::shared_mutex> lock(mtx_cache);

        vec_size = adjacency_list_cache_.at(node_ID_u).size();
        exist_edge = std::find(adjacency_list_cache_.at(node_ID_u).begin(), adjacency_list_cache_.at(node_ID_u).end(), node_ID_v) != adjacency_list_cache_.at(node_ID_u).end();
    }

    // なかった場合
    if (exist_edge == false) {
        {
            // std::lock_guard<std::shared_mutex> lock(mtx_node_cache_[node_ID_u]);
            std::lock_guard<std::shared_mutex> lock(mtx_cache);

            // if (vec.size() == vec_size) { // 隣接リストのサイズが確認時と変わっていない場合、エッジ追加

            //     adjacency_list_cache_.at(node_ID_u).emplace_back(node_ID_v);

            // } else { // // 隣接リストのサイズが確認時と変わっている場合、追加するエッジがすでにないか確認

            //     for (int i = vec_size-1; i < vec.size(); i++) {
            //         if (vec[i] == node_ID_v) exist_edge = true;
            //     }

            //     // なかったらエッジ追加
            //     if (exist_edge == false) {
            //         adjacency_list_cache_.at(node_ID_u).emplace_back(node_ID_v);
            //     }
            // }

            for (int i = vec_size-1; i < adjacency_list_cache_.at(node_ID_u).size(); i++) {
                if (adjacency_list_cache_.at(node_ID_u)[i] == node_ID_v) {
                    exist_edge = true;
                    break;
                }
            }

            // なかったらエッジ追加
            if (exist_edge == false) {
                adjacency_list_cache_.at(node_ID_u).emplace_back(node_ID_v);
            }
        }
    }
}

inline bool Graph::hasServerCacheEdge(const uint32_t& ip, const uint32_t& node_ID_u, const uint32_t& node_ID_v) {
    return servers_who_used_edge_[node_ID_u][node_ID_v].contains(ip);
}

inline void Graph::reset() {
    vertices_IP_cache_.clear();
    adjacency_list_cache_.clear();
    degree_cache_.clear();

    for (auto u : servers_who_used_edge_) {
        for (auto v : u.second) {
            v.second.clear();
        }
    }
}

inline uint32_t Graph::getMyEdgeCount() {
    uint32_t count = 0;
    for (auto lis : adjacency_list_) {
        count += lis.second.size();
    }
    return count;
}

inline uint32_t Graph::getCacheEdgeCount() {
    {
        std::lock_guard<std::shared_mutex> lock(mtx_cache);
        uint32_t count = 0;
        for (auto lis : adjacency_list_cache_) {
            count += lis.second.size();
        }
        return count;
    }
}