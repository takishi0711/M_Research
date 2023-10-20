#include <iostream>
#include <chrono>

using namespace std;

int main() {
    auto now_time = std::chrono::system_clock::now();
    cout << sizeof(now_time) << endl;
    uint64_t a;
    return 0;
}