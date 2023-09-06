#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <algorithm>

#include "param.hpp"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class Graph {

public :

    // グラフファイル読み込み
    void init(const std::string& dir_path, const std::string& hostname, const uint32_t& hostip, const std::vector<uint32_t>& worker_ip_all);

    // 辺の追加
    void addEdge(const std::vector<std::string>& words, const uint32_t& hostip, const std::vector<uint32_t>& worker_ip_all);

    // 自サーバが持ち主となるノードの数を入手
    uint32_t getMyVerticesNum();

    // 自サーバが持ち主となるノード集合を入手
    std::vector<uint32_t> getMyVertices();

    // ノードの持ち主の IP アドレスを入手
    uint32_t getIP(const uint32_t& node_ID);

    // ノードの次数を入手
    uint32_t getDegree(const uint32_t& node_ID);

    // 現在頂点とインデックスを引数にして次の頂点を返す
    uint32_t getNextNode(const uint32_t& current_node, const uint32_t& next_index);

    // ノードの持ち主が自サーバであるか確認
    bool hasVertex(const uint32_t& node_ID);

    // 頂点 u, v を受け取り, u[x] = v の x を返す (index を返す)
    // 頂点 u が自分のサーバのものでない場合は INF を返す
    uint32_t indexOfUV(const uint32_t& node_ID_u, const uint32_t& node_ID_v);
    

private :

    std::unordered_set<uint32_t> my_vertices_; // 自サーバが持ち主となるノード集合
    std::vector<uint32_t> my_vertices_vector_; // 自サーバが持ち主となるノード集合 (配列)
    std::unordered_map<uint32_t, uint32_t> vertices_IP_; // 自サーバが保持しているノードの持ち主の IP アドレス {ノード ID : IP アドレス (ノードの持ち主)}
    std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency_list_; // 自サーバの隣接リスト {ノード ID : 隣接リスト}
    std::unordered_map<uint32_t, uint32_t> degree_; // 自サーバが持ち主となるノードの次数

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成
    std::uniform_real_distribution<double>  rand_double{0, 1.0}; // 0~1のランダムな値

    

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void Graph::init(const std::string& dir_path, const std::string& hostname, const uint32_t& hostip, const std::vector<uint32_t>& worker_ip_all) {
    std::string graph_file_path = dir_path + hostname + ".txt"; // グラフファイルのパス
    std::ifstream reading_file;
    reading_file.open(graph_file_path, std::ios::in);
    std::string reading_line_buffer;
    while (std::getline(reading_file, reading_line_buffer)) { // 空白区切りで word を取り出す
        std::vector<std::string> words; // [ノード 1, ノード 2, ノード 2 のサーバ番号]
        std::stringstream sstream(reading_line_buffer);
        std::string word;
        while (std::getline(sstream, word, ' ')) { // 空白区切りで word を取り出す
            words.emplace_back(word);
        }
        Graph::addEdge(words, hostip, worker_ip_all); // グラフデータに情報追加
    }

    for (uint32_t node : my_vertices_) {
        degree_[node] = adjacency_list_[node].size();
        sort(adjacency_list_[node].begin(), adjacency_list_[node].end()); // 隣接行列はソートした状態に
        my_vertices_vector_.emplace_back(node);
    }
}

inline void Graph::addEdge(const std::vector<std::string>& words, const uint32_t& hostip, const std::vector<uint32_t>& worker_ip_all) {
    uint32_t u = std::stoi(words[0]), v = std::stoi(words[1]);
    my_vertices_.insert(u); // ノード 1 の持ち主は自分
    vertices_IP_[u] = hostip; // ノード 1 の IP アドレスを代入
    vertices_IP_[v] = worker_ip_all[std::stoi(words[2])]; // ノード 2 の IP アドレスを代入
    adjacency_list_[u].emplace_back(v); // ノード １ の隣接リストにノード 2 を追加

}

inline uint32_t Graph::getMyVerticesNum() {
    return my_vertices_.size();
}

inline std::vector<uint32_t> Graph::getMyVertices() {
    return my_vertices_vector_;
}

inline uint32_t Graph::getIP(const uint32_t& node_ID) {
    return vertices_IP_.at(node_ID);
}

inline uint32_t Graph::getDegree(const uint32_t& node_ID) {
    return degree_.at(node_ID);
}

inline uint32_t Graph::getNextNode(const uint32_t& current_node, const uint32_t& next_index) {
    if (degree_[current_node] <= next_index) { // はみ出てる時
        std::cout << "segfault at getNextNode" << std::endl;
        std::uniform_int_distribution<int> rand_int(0, degree_[current_node] - 1);
        return adjacency_list_.at(current_node)[rand_int(mt)];
    }
    return adjacency_list_.at(current_node)[next_index];
}

inline bool Graph::hasVertex(const uint32_t& node_ID) {
    return my_vertices_.contains(node_ID);
}

inline uint32_t Graph::indexOfUV(const uint32_t& node_ID_u, const uint32_t& node_ID_v) {
    if (!hasVertex(node_ID_u)) {
        return INF;
    }
    uint32_t idx = std::lower_bound(adjacency_list_[node_ID_u].begin(), adjacency_list_[node_ID_u].end(), node_ID_v) - adjacency_list_[node_ID_u].begin();
    return idx;
}