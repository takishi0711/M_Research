#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct Graph {

private :
    std::unordered_set<std::string> my_vertices; // 自サーバが持ち主となるノード集合
    std::unordered_map<std::string, int> degree; // 自サーバが持ち主となるノードの次数
    std::unordered_map<std::string, std::string> vertices_IP; // 自サーバが保持しているノードの持ち主の IP アドレス {ノード ID : IP アドレス (ノードの持ち主)}
    std::unordered_map<std::string, std::vector<std::string>> adjacency_list; // 自サーバの隣接リスト {ノード ID : 隣接リスト}

public : 

    // グラフファイル読み込み
    void init(std::string dir_path, std::string hostname, std::string hostip);

    // 辺の追加
    void add_edge(std::vector<std::string>& words, std::string hostip);

    // 自サーバが持ち主となるノード集合を入手
    std::unordered_set<std::string> get_my_vertices();

    // node_ID に対する隣接ノード集合を入手
    std::vector<std::string> get_adjacency_vertices(std::string node_ID);

    // ノードの持ち主の IP アドレスを入手
    std::string get_IP(std::string node_ID);

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline void Graph::init(std::string dir_path, std::string hostname, std::string hostip) {
    std::string graph_file_path = dir_path + hostname + ".txt"; // グラフファイルのパス
    std::ifstream reading_file;
    reading_file.open(graph_file_path, std::ios::in);
    std::string reading_line_buffer;
    while (std::getline(reading_file, reading_line_buffer)) { // 1 行ずつ読み取り
        std::vector<std::string> words; // [ノード 1, ノード 2, ノード 2 の IP アドレス]
        std::stringstream sstream(reading_line_buffer);
        std::string word;
        while (std::getline(sstream, word, ',')) { // カンマ区切りで word を取り出す
            words.push_back(word);
        }
        Graph::add_edge(words, hostip); // グラフデータに情報追加
    }  
}

inline void Graph::add_edge(std::vector<std::string>& words, std::string hostip) { // words : [ノード 1, ノード 2, ノード 2 の IP アドレス]
    my_vertices.insert(words[0]); // ノード 1 の持ち主は自分
    vertices_IP[words[0]] = hostip; // ノード 1 の IP アドレスを代入
    vertices_IP[words[1]] = words[2]; // ノード 2 の IP アドレスを代入
    adjacency_list[words[0]].push_back(words[1]); // ノード １ の隣接リストにノード 2 を追加
    degree[words[0]] = adjacency_list[words[0]].size(); // 次数を更新
}

inline std::unordered_set<std::string> Graph::get_my_vertices() {
    return my_vertices;
}

inline std::vector<std::string> Graph::get_adjacency_vertices(std::string node_ID) {
    return adjacency_list[node_ID];
}

inline std::string Graph::get_IP(std::string node_ID) {
    return vertices_IP[node_ID];
}