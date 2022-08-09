#pragma once

#include <string>
#include <vector>
#include <sstream>


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct RandomWalker {

private :
    std::int32_t RWer_ID; // RWer の ID
    std::int32_t source_node; // 起点ノード
    std::string hostip; // 起点サーバの IP アドレス
    std::int32_t current_node; // 現在のノード
    std::vector<std::int32_t> path; // RWer の経路情報

public :

    // コンストラクタ
    RandomWalker(std::int32_t source_node, std::int32_t RWer_ID, std::string hostip);

    // コンストラクタ 2
    RandomWalker() {}

    // コンストラクタ 3 (デシリアライズ)
    RandomWalker(std::string message);

    // RWer の ID を入手
    std::int32_t get_RWer_ID();

    // 起点ノードを入手
    std::int32_t get_source_node();

    // 起点サーバの IP アドレスを入手
    std::string get_hostip();

    // 現在のノードを入手
    std::int32_t get_current_node();

    // RWer の経路情報を入手
    std::vector<std::int32_t> get_path();

    // RWer の現在ノードを更新
    void update_RWer(std::int32_t next_node);

    // シリアライズ用の関数
    std::string serialize();
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline RandomWalker::RandomWalker(std::int32_t source_node, std::int32_t RWer_ID, std::string hostip) {
    this->RWer_ID = RWer_ID;
    this->source_node = source_node;
    this->hostip = hostip;
    this->current_node = source_node; 
    this->path.push_back(source_node);
}

inline std::int32_t RandomWalker::get_RWer_ID() {
    return RWer_ID;
}

inline std::int32_t RandomWalker::get_source_node() {
    return source_node;
}

inline std::string RandomWalker::get_hostip() {
    return hostip;
}

inline std::int32_t RandomWalker::get_current_node() {
    return current_node;
}

inline std::vector<std::int32_t> RandomWalker::get_path() {
    return path;
}

inline void RandomWalker::update_RWer(std::int32_t next_node) {
    current_node = next_node;
    path.push_back(next_node);
}

inline std::string RandomWalker::serialize() {
    std::string message(217, '0');
    int now_index = 0;
    union { std::int32_t a; char b[4];} _4byte;

    // RWer_ID
    _4byte.a = RWer_ID;
    for (int i = 0; i < 4; i++) {
        message[now_index + i] = _4byte.b[i];
    }
    now_index += 4;

    // source_node
    _4byte.a = source_node;
    for (int i = 0; i < 4; i++) {
        message[now_index + i] = _4byte.b[i];
    }
    now_index += 4;

    // hostip
    std::stringstream sstream(hostip);
    std::string word;
    while (std::getline(sstream, word, '.')) {
        message[now_index] = char(std::stoi(word));
        now_index++;
    }

    // current_node
    _4byte.a = current_node;
    for (int i = 0; i < 4; i++) {
        message[now_index + i] = _4byte.b[i];
    }
    now_index += 4;

    // path length
    int len = path.size();
    message[now_index] = char(len);
    now_index++;

    // path
    for (int i = 0; i < std::min(len, 50); i++) {
        _4byte.a = path[i];
        for (int j = 0; j < 4; j++) {
            message[now_index + j] = _4byte.b[j];
        }
        now_index += 4;        
    }

    return message;
}

inline RandomWalker::RandomWalker(std::string message) {
    int now_index = 0;
    union { std::int32_t a; char b[4];} _4byte;

    // RWer_ID
    for (int i = 0; i < 4; i++) {
        _4byte.b[i] = message[now_index + i];
    }
    now_index += 4;
    RWer_ID = _4byte.a;

    // source_node
    for (int i = 0; i < 4; i++) {
        _4byte.b[i] = message[now_index + i];
    }
    now_index += 4;
    source_node = _4byte.a;

    // hostip
    std::string ip;
    for (int i = 0; i < 4; i++) {
        ip += std::to_string(int(message[now_index + i]));
        ip += '.';
    }
    ip.pop_back();
    now_index += 4;
    hostip = ip;

    // current_node
    for (int i = 0; i < 4; i++) {
        _4byte.b[i] = message[now_index + i];
    }
    now_index += 4;
    current_node = _4byte.a;

    // path length
    int len = int(message[now_index]);
    now_index++;
    
    // path
    for (int i = 0; i < std::min(len, 50); i++) {
        for (int i = 0; i < 4; i++) {
            _4byte.b[i] = message[now_index + i];
        }
        now_index += 4;
        path.push_back(_4byte.a);
    }
}