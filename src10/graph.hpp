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

    // ノードに対するキャッシュのIPアドレスを入手
    uint32_t getCacheIP(const uint32_t& node_ID);

    // エッジをキャッシュに追加
    void addEdgeToCache(const uint32_t& node_ID_u, const uint32_t& hostip_u, const uint32_t& degree_u, const uint32_t& node_ID_v, const uint32_t& hostip_v, const uint32_t& degree_v, const std::unordered_set<uint32_t>& ip_set);

    // 引数のサーバが引数のエッジをキャッシュとして持っているかどうか
    bool hasServerCacheEdge(const uint32_t& ip, const uint32_t& node_ID_u, const uint32_t& node_ID_v);
    
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

    // キャッシュのmutex(ノードごと)
    std::unordered_map<uint32_t, std::shared_mutex> mtx_node_cache_;
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
            words.push_back(word);
        }
        Graph::addEdge(words, hostip); // グラフデータに情報追加
    }  
    for (uint32_t node : my_vertices_) {
        my_vertices_vector_.push_back(node);
    }
}

inline void Graph::addEdge(const std::vector<std::string>& words, const uint32_t& hostip) { // words : [ノード 1, ノード 2, ノード 2 の IP アドレス]
    uint32_t u = std::stoi(words[0]), v = std::stoi(words[1]);
    my_vertices_.insert(u); // ノード 1 の持ち主は自分
    vertices_IP_[u] = hostip; // ノード 1 の IP アドレスを代入
    vertices_IP_[v] = inet_addr(words[2].c_str()); // ノード 2 の IP アドレスを代入
    adjacency_list_[u].push_back(v); // ノード １ の隣接リストにノード 2 を追加
    degree_[u] = adjacency_list_[u].size(); // 次数を更新

    if (vertices_IP_[v] != hostip) { // キャッシュノードの mutex を登録
        mtx_node_cache_[v];
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
    return adjacency_list_[node_ID];
}

inline uint32_t Graph::getIP(const uint32_t& node_ID) {
    return vertices_IP_[node_ID];
}

inline uint32_t Graph::getDegree(const uint32_t& node_ID) {
    return degree_[node_ID];
}

inline bool Graph::hasVertex(const uint32_t& node_ID) {
    return my_vertices_.contains(node_ID);
}

inline void Graph::nodeCache(const uint32_t& node_ID, const uint32_t& hostip, const uint32_t& degree) {
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
        std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);

        if (vertices_IP_cache_.contains(node_ID)) exist_node_data = true;
    }

    if (exist_node_data == false) { // 存在しない場合のみ占有ロック
        {
            std::lock_guard<std::shared_mutex> lock(mtx_node_cache_[node_ID]);

            vertices_IP_cache_[node_ID] = hostip;
            degree_cache_[node_ID] = degree;
            adjacency_list_cache_[node_ID].reserve(degree);
        }
    }
}

inline bool Graph::hasCacheDegree(const uint32_t& node_ID) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);

        return degree_cache_.contains(node_ID);
    }
}

inline uint32_t Graph::getCacheDegree(const uint32_t& node_ID) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);

        return degree_cache_[node_ID];
    }
}

inline std::vector<uint32_t> Graph::getCacheAdjacencyList(const uint32_t& node_ID) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);

        return adjacency_list_cache_[node_ID];
    }
}

inline uint32_t Graph::getCacheIP(const uint32_t& node_ID) {
    {
        std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID]);

        return vertices_IP_cache_[node_ID];
    }
}

inline void Graph::addEdgeToCache(const uint32_t& node_ID_u, const uint32_t& hostip_u, const uint32_t& degree_u, const uint32_t& node_ID_v, const uint32_t& hostip_v, const uint32_t& degree_v, const std::unordered_set<uint32_t>& ip_set) {
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
        std::shared_lock<std::shared_mutex> lock(mtx_node_cache_[node_ID_u]);

        std::vector<uint32_t> vec = adjacency_list_cache_[node_ID_u];
        vec_size = vec.size();
        exist_edge = std::find(vec.begin(), vec.end(), node_ID_v) != vec.end();
    }

    // なかった場合
    if (exist_edge == false) {
        {
            std::lock_guard<std::shared_mutex> lock(mtx_node_cache_[node_ID_u]);

            std::vector<uint32_t> vec = adjacency_list_cache_[node_ID_u];
            if (vec.size() == vec_size) { // 隣接リストのサイズが確認時と変わっていない場合、エッジ追加

                adjacency_list_cache_[node_ID_u].push_back(node_ID_v);

            } else { // // 隣接リストのサイズが確認時と変わっている場合、追加するエッジがすでにないか確認

                for (int i = vec_size-1; i < vec.size(); i++) {
                    if (vec[i] == node_ID_v) exist_edge = true;
                }

                // なかったらエッジ追加
                if (exist_edge == false) {
                    adjacency_list_cache_[node_ID_u].push_back(node_ID_v);
                }
            }
        }
    }
}

inline bool Graph::hasServerCacheEdge(const uint32_t& ip, const uint32_t& node_ID_u, const uint32_t& node_ID_v) {
    return servers_who_used_edge_[node_ID_u][node_ID_v].contains(ip);
}