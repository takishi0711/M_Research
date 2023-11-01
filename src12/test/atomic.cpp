#include <iostream>
#include <thread>
#include <atomic>

using namespace std;

int main() {
    atomic<uint32_t> num = 0;
    // uint32_t num = 0;

    thread th1([&]() {
        for (int i = 0; i < 1000000; i++) num++;
    });

    thread th2([&]() {
        for (int i = 0; i < 1000000; i++) num++;
    });

    th1.join();
    th2.join();

    cout << num << endl;

    return 0;
}