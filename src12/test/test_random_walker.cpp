#include <iostream>
#include <unordered_set>
#include <memory>

using namespace std;


#include "../random_walker.hpp"

int main() {
    // スライドの例の経路を RWer に入力してみる
    {
        // 1, 通常の遷移
        std::cout << "normal" << std::endl;
        std::cout << std::endl;
        // RWer_id = 1000
        // {1, 1, 1}, (1, 3, _, _)
        RandomWalker RWer(1, 3, 1000, 1, 5);
        RWer.printRWer();

        // {1, 2, 1}, (1, 3, _, _), (4, 2, 1, 0)
        RWer.updateRWer(4, 1, 0, 1, 0);
        RWer.setCurrentDegree(2);
        RWer.printRWer();

        // {1, 2, 0}, (1, 3, _, _), (4, 2, 1, 0)
        // {2, 1, 1}, (10, 4, 1, 0)
        RWer.updateRWer(10, 2, 0, 1, 0);
        RWer.inputSendFlag(true);
        RWer.setCurrentDegree(4);
        RWer.printRWer();

        // {1, 2, 0}, (1, 3, _, _), (4, 2, 1, 0)
        // {2, 1, 1}, (10, 4, 1, 0)
        // {3, 1, 1}, (11, 3, 3, 0)
        RWer.updateRWer(11, 3, 0, 3, 0);
        RWer.inputSendFlag(true);
        RWer.setCurrentDegree(3);
        RWer.printRWer();

        // {1, 2, 0}, (1, 3, _, _), (4, 2, 1, 0)
        // {2, 1, 1}, (10, 4, 1, 0)
        // {3, 2, 1}, (11, 3, 3, 0), (13, 2, 2, 0)
        RWer.updateRWer(13, 3, 0, 2, 0);
        RWer.setCurrentDegree(2);
        RWer.printRWer();
    }


    // 2, キャッシュを使った遷移 
    std::cout << "cache" << std::endl;
    std::cout << std::endl;
    // 10->11 だけはわかっている. 11->13 はわかっていないが, 11 の次数はわかっている状態
    // つまり, 4->10 の通信はスキップされるが, 11 への通信はスキップされない
    {

        RandomWalker RWer(1, 3, 1000, 1, 5);

        // {1, 1, 1}, (1, 3, _, _)
        RWer.printRWer();

        RWer.updateRWer(4, 1, 0, 1, 0);

        RWer.setCurrentDegree(2);

        // {1, 2, 1}, (1, 3, _, _), (4, 2, 1, 0)
        RWer.printRWer();

        RWer.updateRWer(10, 2, 0, 1, 0);

        RWer.setCurrentDegree(4);

        // {1, 2, 0}, (1, 3, _, _), (4, 2, 1, 0)
        // {2, 1, 0}, (10, 4, 1, 0)
        RWer.printRWer();

        // キャッシュデータを使って遷移 (10->11)
        RWer.updateRWer(11, 3, 0, 3, 1001002003);

        // {1, 2, 0}, (1, 3, _, _), (4, 2, 1, 0)
        // {2, 1, 0}, (10, 4, 1, 0)
        // {3, 1, 0}, (11, 0, 3, 1001002003)
        RWer.printRWer();

        RWer.setIndex(2);
        RWer.inputSendFlag(true);

        // ここで通信が発生

        RWer.printRWer();

        RWer.setCurrentDegree(3);

        // 11->10 の index を登録
        RWer.setPrevIndex(0);

        RWer.updateRWer(13, 3, 0, RWer.getNextIndex(), 0);

        RWer.setCurrentDegree(2);

        // {1, 2, 0}, (1, 3, _, _), (4, 2, 1, 0)
        // {2, 1, 0}, (10, 4, 1, 0)
        // {3, 2, 1}, (11, 3, 3, 1001002003), (13, 2, 2, 0)
        RWer.printRWer();
    }

    // RWer のシリアライズとデシリアライズ
    {
        RandomWalker RWer(1, 3, 1000, 1, 5);
        RWer.updateRWer(4, 1, 0, 1, 0);
        RWer.setCurrentDegree(2);
        RWer.updateRWer(10, 2, 0, 1, 0);
        RWer.inputSendFlag(true);
        RWer.setCurrentDegree(4);
        RWer.updateRWer(11, 3, 0, 3, 0);
        RWer.inputSendFlag(true);
        RWer.setCurrentDegree(3);
        RWer.updateRWer(13, 3, 0, 2, 0);
        RWer.setCurrentDegree(2);

        char message[10000];

        RWer.writeMessage(message);

        std::cout << "write" << std::endl;

        unique_ptr<RandomWalker> RWer2(new RandomWalker(message));

        RWer.printRWer();
        RWer2->printRWer();
    }

    // getHostGroup のテスト
    {
        RandomWalker RWer(1, 3, 1000, 1, 5);
        RWer.updateRWer(4, 1, 0, 1, 0);
        RWer.setCurrentDegree(2);
        RWer.updateRWer(10, 2, 0, 1, 0);
        RWer.inputSendFlag(true);
        RWer.setCurrentDegree(4);
        RWer.updateRWer(11, 3, 0, 3, 0);
        RWer.inputSendFlag(true);
        RWer.setCurrentDegree(3);
        RWer.updateRWer(13, 3, 0, 2, 0);
        RWer.setCurrentDegree(2);

        auto host_set = RWer.getHostGroup();

        for (auto id : host_set) std::cout << id << " ";
        std::cout << std::endl;
    }

    // たくさん生成してみる
    {
        vector<unique_ptr<RandomWalker>> vec;

        for (int i = 0; i < 100; i++) {
            std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(10, 10, 10, 10, 100));
            vec.push_back(move(RWer_ptr));
        }

        std::unique_ptr<RandomWalker> RWer_ptr(new RandomWalker(10, 10, 10, 10, 100));
        cout << sizeof(*RWer_ptr) << endl;
    }
    return 0;
}