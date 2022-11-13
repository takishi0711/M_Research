#pragma once

#include <string>
#include <vector>
#include <sstream>


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalker {

public :

    // コンストラクタ
    RandomWalker(const uint32_t source_node, const uint32_t RWer_id, const std::string& hostip);

    // コンストラクタ 2
    RandomWalker() {}

    // コンストラクタ 3 (デシリアライズ)
    RandomWalker(char* message, int& message_index);

    // RWer の ID を入手
    uint32_t getRWerId();

    // 起点ノードを入手
    uint32_t getSourceNode();

    // 起点サーバの IP アドレスを入手
    std::string getHostip();

    // 現在のノードを入手
    uint32_t getCurrentNode();

    // RWer の経路情報を入手
    std::vector<uint32_t> getPath();

    // RWer の end_flag を入手
    bool getEndFlag();

    // RWer の現在ノードを更新
    void updateRWer(const uint32_t next_node);

    // RWer を終了させる
    void endRWer();

    // シリアライズ用の関数
    void serialize(char* message, int& message_index);

private :
    uint32_t RWer_id_; // RWer の ID
    uint32_t source_node_; // 起点ノード
    std::string hostip_; // 起点サーバの IP アドレス
    uint32_t current_node_; // 現在のノード
    std::vector<uint32_t> path_; // RWer の経路情報
    bool end_flag_ = false; // RW が終了しているか

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


inline RandomWalker::RandomWalker(const uint32_t source_node, const uint32_t RWer_id, const std::string& hostip) {
    RWer_id_ = RWer_id;
    source_node_ = source_node;
    hostip_ = hostip;
    current_node_ = source_node; 
    path_.push_back(source_node);
}

inline uint32_t RandomWalker::getRWerId() {
    return RWer_id_;
}

inline uint32_t RandomWalker::getSourceNode() {
    return source_node_;
}

inline std::string RandomWalker::getHostip() {
    return hostip_;
}

inline uint32_t RandomWalker::getCurrentNode() {
    return current_node_;
}

inline std::vector<uint32_t> RandomWalker::getPath() {
    return path_;
}

inline bool RandomWalker::getEndFlag() {
    return end_flag_;
}

inline void RandomWalker::updateRWer(uint32_t next_node) {
    current_node_ = next_node;
    path_.push_back(next_node);
}

inline void RandomWalker::endRWer() {
    end_flag_ = true;
}

inline void RandomWalker::serialize(char* message, int& message_index) {
    union { uint32_t a; char b[4];} _4byte;

    // RWer_id
    _4byte.a = RWer_id_;
    for (int i = 0; i < 4; i++) {
        message[message_index + i] = _4byte.b[i];
    }
    message_index += 4;

    // source_node
    _4byte.a = source_node_;
    for (int i = 0; i < 4; i++) {
        message[message_index + i] = _4byte.b[i];
    }
    message_index += 4;

    // hostip
    std::stringstream sstream(hostip_);
    std::string word;
    while (std::getline(sstream, word, '.')) {
        message[message_index] = char(std::stoi(word));
        message_index++;
    }

    // current_node
    _4byte.a = current_node_;
    for (int i = 0; i < 4; i++) {
        message[message_index + i] = _4byte.b[i];
    }
    message_index += 4;

    // path length
    int len = path_.size();
    message[message_index] = char(len);
    message_index++;

    // path
    for (int i = 0; i < std::min(len, 50); i++) {
        _4byte.a = path_[i];
        for (int j = 0; j < 4; j++) {
            message[message_index + j] = _4byte.b[j];
        }
        message_index += 4;        
    }

    // end_flag
    if (end_flag_) message[message_index] = '1';
    else message[message_index] = '0';
    message_index++;
}

inline RandomWalker::RandomWalker(char* message, int& message_index) {
    union { uint32_t a; char b[4];} _4byte;

    // RWer_id
    for (int i = 0; i < 4; i++) {
        _4byte.b[i] = message[message_index + i];
    }
    message_index += 4;
    RWer_id_ = _4byte.a;

    // source_node
    for (int i = 0; i < 4; i++) {
        _4byte.b[i] = message[message_index + i];
    }
    message_index += 4;
    source_node_ = _4byte.a;

    // hostip
    std::string ip;
    for (int i = 0; i < 4; i++) {
        ip += std::to_string(int(message[message_index + i]));
        ip += '.';
    }
    ip.pop_back();
    message_index += 4;
    hostip_ = ip;

    // current_node
    for (int i = 0; i < 4; i++) {
        _4byte.b[i] = message[message_index + i];
    }
    message_index += 4;
    current_node_ = _4byte.a;

    // path length
    int len = int(message[message_index]);
    message_index++;
    
    // path
    for (int i = 0; i < std::min(len, 50); i++) {
        for (int i = 0; i < 4; i++) {
            _4byte.b[i] = message[message_index + i];
        }
        message_index += 4;
        path_.push_back(_4byte.a);
    }

    // end_flag
    if (message[message_index] == '1') end_flag_ = true;
    else end_flag_ = false;
}