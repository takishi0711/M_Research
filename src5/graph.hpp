#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <arpa/inet.h>

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
    
private :

    std::unordered_set<uint32_t> my_vertices_; // 自サーバが持ち主となるノード集合
    std::vector<uint32_t> my_vertices_vector_; // 自サーバが持ち主となるノード集合 (配列)
    std::unordered_map<uint32_t, uint32_t> vertices_IP_; // 自サーバが保持しているノードの持ち主の IP アドレス {ノード ID : IP アドレス (ノードの持ち主)}
    std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency_list_; // 自サーバの隣接リスト {ノード ID : 隣接リスト}
    std::unordered_map<uint32_t, uint32_t> degree_; // 自サーバが持ち主となるノードの次数

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
    my_vertices_.insert(std::stoi(words[0])); // ノード 1 の持ち主は自分
    vertices_IP_[std::stoi(words[0])] = hostip; // ノード 1 の IP アドレスを代入
    vertices_IP_[std::stoi(words[1])] = inet_addr(words[2].c_str()); // ノード 2 の IP アドレスを代入
    adjacency_list_[std::stoi(words[0])].push_back(std::stoi(words[1])); // ノード １ の隣接リストにノード 2 を追加
    degree_[std::stoi(words[0])] = adjacency_list_[std::stoi(words[0])].size(); // 次数を更新
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