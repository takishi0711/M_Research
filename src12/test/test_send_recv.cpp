#include <iostream>
#include <memory>
#include <cassert>

using namespace std;

#include "../random_walker.hpp"
#include "../message_queue.hpp"
#include "../param.hpp"

unique_ptr<RandomWalker> createRWer(int num) {
    return make_unique<RandomWalker>(num, num, num, num, num);
}

int main() {
    {
        MessageQueue<RandomWalker> que;

        unique_ptr<RandomWalker> RWer_ptr_test(new RandomWalker(100, 100, 100, 100, 100));
        que.push(move(RWer_ptr_test));

        for (int i = 1; i <= 3; i++) {
            que.push(move(createRWer(i)));
        }

        // auto r = que.pop();
        // r->printRWer();


        cout << "queue_size: " << que.getSize() << endl;

        // 送信部分

        // メッセージ宣言
        char message[MESSAGE_MAX_LENGTH];

        // キューからRWerを取ってきてメッセージに詰める
        uint16_t RWer_count = 0;
        uint32_t now_length = que.pop(message + sizeof(uint8_t) + sizeof(uint16_t), MESSAGE_MAX_LENGTH, RWer_count);

        cout << 1 << endl;

        // メッセージのヘッダ情報を書き込む
        // バージョン: 4bit (0), 
        // メッセージID: 4bit (2),
        // メッセージに含まれるRWerの個数: 16bit
        uint8_t ver_id = 2;
        memcpy(message, &ver_id, sizeof(uint8_t));
        memcpy(message + sizeof(uint8_t), &RWer_count, sizeof(uint16_t));
        now_length += sizeof(uint8_t) + sizeof(uint16_t);

        cout << "RWer_count: " << RWer_count << endl;
        cout << "now_length: " << now_length << endl;

        // 受信部分
        assert((ver_id & MASK_MESSEGEID) == 2);

        // message に入っている RWer の数を確認
        int idx = sizeof(uint8_t);
        RWer_count = *(uint16_t*)(message + idx); idx += sizeof(uint16_t);

        for (int i = 0; i < RWer_count; i++) {
            std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(message + idx));
            idx += RWer_ptr->getRWerSize();
            // // メッセージキューに push
            // receive_queue_[port_num].push(std::move(RWer_ptr));
            RWer_ptr->printRWer();
        }
    }

    return 0;
}