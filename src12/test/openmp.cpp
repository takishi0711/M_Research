#include <iostream>
#include <omp.h>
#include <thread>
#include <chrono>
#include <vector>
#include <numeric>

using namespace std;

int main() {
    // uint32_t RWer_num_all = 10;

    // uint32_t num_threads = 3;
    // omp_set_num_threads(num_threads);

    // uint32_t RWer_id = 0;
    // #pragma omp parallel private(RWer_id)
    // {
    //     RWer_id = omp_get_thread_num();
    //     while (1) {
    //         cout << RWer_id << endl;
    //         RWer_id += num_threads;
    //         if (RWer_id >= RWer_num_all) break;
    //         std::this_thread::sleep_for(std::chrono::seconds(3));
    //     }
    // }

    vector<int> vec(10);
    iota(vec.begin(), vec.end(), 0);
    uint32_t vec_size = 10;
    uint32_t progress = 0;
    uint32_t WORK_STEP = 3;
    #pragma omp parallel num_threads(3)
    {  
        uint32_t worker_id = omp_get_thread_num();
        uint32_t next_idx;
        while ((next_idx =  __sync_fetch_and_add(&progress, WORK_STEP)) < vec_size) {
            uint32_t begin_idx = next_idx;
            uint32_t end_idx = min(next_idx + WORK_STEP, vec_size);
            for (uint32_t idx = begin_idx; idx < end_idx; idx++) {
                cout << worker_id << " " << vec[idx] << endl;
            }
        }

    }
    return 0;
}