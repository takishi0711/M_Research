#include <iostream>
#include <cstring>

using namespace std;

#include "../param.hpp"

int main() {
    int MESSAGE_LENGTH = 250;
    uint32_t hostip_ = 100;
    uint32_t RW_execution_num_ = 10;
    char message[MESSAGE_LENGTH];
    // メッセージのヘッダ情報を書き込む
    // バージョン: 4bit (0), 
    // メッセージID: 4bit (3),
    uint8_t ver_id = 3; 
    memcpy(message, &ver_id, sizeof(ver_id));
    memcpy(message + sizeof(ver_id), &hostip_, sizeof(hostip_));
    memcpy(message + sizeof(ver_id) + sizeof(hostip_), &RW_execution_num_, sizeof(RW_execution_num_));

    uint8_t ver_id2 = *(uint8_t*)message;
    uint32_t startmanager_ip = *(uint32_t*)(message + sizeof(ver_id));
    uint32_t num_RWer = *(uint32_t*)(message + sizeof(ver_id) + sizeof(startmanager_ip));
    cout << (uint32_t)ver_id2 << endl;
    cout << (ver_id & MASK_MESSEGEID) << " " << startmanager_ip << " " << num_RWer << endl;
    return 0;
}