#pragma once

#include <iostream>

// ver_id_ のマスク
const uint32_t MASK_VER = (1<<7) + (1<<6) + (1<<5) + (1<<4);
const uint32_t MASK_MESSEGEID = (1<<3) + (1<<2) + (1<<1) + (1<<0);

// message_id_ の値
const uint32_t ALIVE = 0;
const uint32_t DEAD = 1;
const uint32_t RWERS = 2;
const uint32_t START_EXP = 3;
const uint32_t CACHE_GEN = 4;
const uint32_t END_EXP = 5;
const uint32_t DEAD_SEND = 6;
const uint32_t DUMMY = 7;

// パラメタ
const uint32_t RECV_PORT = 4;
const uint32_t RECV_PER_PORT = 1;
uint32_t PROC_MESSAGE = 15;
const uint32_t GENERATE_RWER = 15;
const uint32_t GENERATE_RWER_CACHE = 4;
uint32_t PROC_MESSAGE_CACHE = 10;
const uint32_t WORK_STEP = 32;
const uint32_t SEND2_THREAD_NUM = 4;
uint32_t SEND_QUEUE_NUM = 7;

// 十分に大きな値
const uint32_t INF = 2001002003;

// パラメタ (メッセージ長)
const uint32_t MESSAGE_MAX_LENGTH = 8950;

// キャッシュサイズの最大値
const uint32_t MAX_CACHE_SIZE = 0;
uint32_t MY_EDGE_NUM = 0;

// RW の実行ステップ (最初の RW_STEP_1 % を実行した後少しスリープ)
const uint32_t RW_STEP_1 = 1;

// RW の実行ステップ (それぞれの頂点から RW_STEP_2 回実行した後少しスリープ)
const uint32_t RW_STEP_2 = 10;

// RW の実行ステップ (RW_STEP_3 回実行した後少しスリープ)
const uint32_t RW_STEP_3 = 500000;

// checkRWer をするかどうか
bool CHECK_FLAG = false;

// データ構造の初期化用
const uint64_t VERTEX_SIZE = 5000000;

// メインの実験が始まっているかどうか
bool MAIN_EX = true;

// procMessage の中断用フラグ
bool PROC_FLAG = true;

// cache 用の実行を続けるためのフラグ
bool CACHE_GEN_FLAG = true;
const uint64_t MAX_RWER_NUM_FOR_CACHE = 1000000000; 

// RWer 生成の sleep を使うかどうか
const bool GENERATE_SLEEP_FLAG = true;
// RWer 生成の sleep 時間 (s)
const uint32_t GENERATE_SLEEP = 4;