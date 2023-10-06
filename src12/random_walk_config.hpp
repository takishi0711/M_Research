#pragma once

#include <random>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class RandomWalkConfig {

public :

    // RW の実行回数を設定
    void setNumberOfRWExecution(const uint32_t& RW_num);

    // RW の実行回数を入手
    uint32_t getNumberOfRWExecution();

    // RW の終了確率を入手
    double getAlpha();

    // α の値から
    uint16_t getRWerLife();

private :
    uint32_t number_of_RW_execution_ = 10000; // RW の実行回数
    double alpha_ = 0.02; // RW の終了確率

    // 乱数関連
    std::mt19937 mt{std::random_device{}()}; // メルセンヌ・ツイスタを用いた乱数生成
    std::uniform_real_distribution<double>  rand_double{0, 1.0}; // 0~1のランダムな値

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void RandomWalkConfig::setNumberOfRWExecution(const uint32_t& RW_num) {
    number_of_RW_execution_ = RW_num;
}

inline uint32_t RandomWalkConfig::getNumberOfRWExecution() {
    return number_of_RW_execution_;
}

inline double RandomWalkConfig::getAlpha() {
    return alpha_;
}

inline uint16_t RandomWalkConfig::getRWerLife() {
    uint16_t life = 0;
    while (rand_double(mt) > alpha_) life++;
    return life;
}

