#include <iostream>
#include <chrono>

using namespace std;

#include "../random_walk_config.hpp"

int main() {
    RandomWalkConfig RW_config_;
    
    std::chrono::system_clock::time_point  start, end;
    start = std::chrono::system_clock::now();
    // 処理
    // int sum = 0;
    // for (long long i = 0; i < 1001002003; i++) {
    //     sum += 1;
    // }
    int num = 1000;
    double sum = 0;
    for (int i = 0; i < num; i++) {
        uint16_t RWer_life = RW_config_.getRWerLife();
        std::cout << RWer_life << std::endl;
        sum += RWer_life;
    }
    cout << sum / num << endl;
    end = std::chrono::system_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    elapsed /= 1000000;
    // std::cout << elapsed << std::endl;
    printf("%.10f", elapsed);
    return 0;
}