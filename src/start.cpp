#include "start_manager.hpp"

// 「実験開始の合図を全サーバに送り、実験の終了を観測する」サーバ
int main(int argc, char *argv[]) {
    StartManager start;

    start.send_start();
}