#include <iostream>
#include <string>
#include <fstream>
#include <thread>

#include "start_manager.hpp"

// 「実験開始の合図を全サーバに送り、実験の終了を観測する」サーバ
int main(int argc, char *argv[]) {


    // 分割数
    int split_num = 0;
    std::cout << "分割数？" << std::endl;
    std::cin >> split_num;

    // 実行回数受付
    int cnt = 0;
    std::cout << "実行回数？" << std::endl;
    std::cin >> cnt;

    // 実験結果を書き込むファイル名受付
    std::string filename;
    std::cout << "ファイル名？" << std::endl;
    std::cin >> filename;

    std::string filename_time = "../output/" + filename + "_time.txt";
    std::string filename_rerun = "../output/" + filename + "_rerun.txt";

    std::ofstream ofs_time, ofs_rerun;
    ofs_time.open(filename_time, std::ios::app);
    ofs_rerun.open(filename_rerun, std::ios::app);

    // RW 実行回数受付
    int32_t RW_num = 0;
    std::cout << "RW実行回数？" << std::endl;
    std::cin >> RW_num;

    // 待機時間
    int32_t wait_time = 0;
    std::cout << "待機時間？" << std::endl;
    std::cin >> wait_time;

    StartManager start(split_num);

    for (int i = 0; i < cnt; i++) {
        start.sendStart(ofs_time, ofs_rerun, RW_num);

        // if (RW_num == 100000) {
        //     std::this_thread::sleep_for(std::chrono::seconds(200));
        // } else {
        //     std::this_thread::sleep_for(std::chrono::seconds(30));
        // }

        std::this_thread::sleep_for(std::chrono::seconds(wait_time));
        
        start.sendEnd(ofs_time, ofs_rerun);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}