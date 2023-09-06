#pragma once

#include <iostream>

// パラメタ (スレッド数)
const uint32_t SEND_RECV_PORT = 8;
const uint32_t GENERATE_RWER = 1;

// 十分に大きな値
const uint32_t INF = 1001002003;

// RWer の最大経路長
const uint32_t MAX_PATH_LENGTH = 50;

// 生存 RWer の最大数
const uint32_t MAX_SURVIVING_RWER = 500;

// パラメタ (メッセージ長)
const uint32_t MESSAGE_MAX_LENGTH = 1450;

// パラメタ (メッセージ終了判定)
const uint8_t MESSAGE_END = 100;

// キャッシュサイズの最大値
const uint32_t MAX_CACHE_SIZE = 100000;