#include <iostream>

using namespace std;

// 始点の数
uint32_t START_VERTEX_NUM = 1000;

// １頂点あたりの RW 実行回数
uint32_t RW_NUM = 50000;

// 終了確率α
double ALPHA = 0.15;

// スレッド数
uint32_t THREADS_NUM = 20;

// RWer が落ちる割合 (%)
double DROP_PROB = 15;
