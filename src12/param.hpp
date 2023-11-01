#pragma once

#include <iostream>

// 型エイリアス
using vertex_id_t = uint64_t;
using walker_id_t = uint32_t;
using index_t = uint64_t;

// ver_id_ のマスク
const uint32_t MASK_VER = (1<<7) + (1<<6) + (1<<5) + (1<<4);
const uint32_t MASK_MESSEGEID = (1<<3) + (1<<2) + (1<<1) + (1<<0);

// message_id_ の値
const uint32_t ALIVE = 0;
const uint32_t DEAD = 1;
const uint32_t RWERS = 2;
const uint32_t START_EXP = 3;
const uint32_t END_EXP = 4;
const uint32_t DEAD_SEND = 5;

// パラメタ (スレッド数)
const uint32_t RECV_PORT = 4;
const uint32_t RECV_PER_PORT = 1;
uint32_t PROC_MESSAGE_PER_PORT = 3;
const uint32_t GENERATE_RWER = 4;
const uint32_t WORK_STEP = 32;

// 十分に大きな値
const uint32_t INF = 1001002003;

// RWer の最大経路長
const uint32_t MAX_PATH_LENGTH = 50;

// 生存 RWer の最大数
const uint32_t MAX_SURVIVING_RWER = 1000;

// パラメタ (メッセージ長)
const uint32_t MESSAGE_MAX_LENGTH = 8950;

// キャッシュサイズの最大値
const uint32_t MAX_CACHE_SIZE = 1000000;

// RW の実行ステップ (最初の RW_STEP_1 % を実行した後少しスリープ)
const uint32_t RW_STEP_1 = 1;

// RW の実行ステップ (それぞれの頂点から RW_STEP_2 回実行した後少しスリープ)
const uint32_t RW_STEP_2 = 100;

// cache を使うかどうか
bool USE_CACHE = true;

// RWer 生成の sleep を使うかどうか
const bool GENERATE_SLEEP_FLAG = true;
// RWer 生成の sleep 時間 (s)
const uint32_t GENERATE_SLEEP = 2;

// proc_message 内で並列化するかどうかの閾値
const uint32_t OMP_PARALLEL_THRESHOLD = 100;

