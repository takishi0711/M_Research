#pragma once

#include <string>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct RandomWalker {

private :
    std::string RWer_ID; // RWer の ID
    std::string source_node; // 起点ノード
    std::string hostip; // 起点サーバの IP アドレス
    std::string current_node; // 現在のノード
    std::vector<std::string> path; // RWer の経路情報

public :

    // コンストラクタ
    RandomWalker(std::string source_node, int RWer_ID, std::string hostip);

    // コンストラクタ 2
    RandomWalker() {}

    // RWer の ID を入手
    int get_RWer_ID();

    // 起点ノードを入手
    std::string get_source_node();

    // 起点サーバの IP アドレスを入手
    std::string get_hostip();

    // 現在のノードを入手
    std::string get_current_node();

    // RWer の経路情報を入手
    std::vector<std::string> get_path();

    // RWer の現在ノードを更新
    void update_RWer(std::string next_node);

    // シリアライズ用の関数
    template<class Archive>
    void serialize(Archive& archive) {
        archive(RWer_ID, source_node, hostip, current_node, path);
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline RandomWalker::RandomWalker(std::string source_node, int RWer_ID, std::string hostip) {
    this->RWer_ID = std::to_string(RWer_ID);
    this->source_node = source_node;
    this->hostip = hostip;
    this->current_node = source_node; 
    this->path.push_back(source_node);
}

inline int RandomWalker::get_RWer_ID() {
    return std::stoi(RWer_ID);
}

inline std::string RandomWalker::get_source_node() {
    return source_node;
}

inline std::string RandomWalker::get_hostip() {
    return hostip;
}

inline std::string RandomWalker::get_current_node() {
    return current_node;
}

inline std::vector<std::string> RandomWalker::get_path() {
    return path;
}

inline void RandomWalker::update_RWer(std::string next_node) {
    current_node = next_node;
    path.push_back(next_node);
}
