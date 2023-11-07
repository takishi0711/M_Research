#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <unordered_map>
#include <fstream>

#include "type.hpp"
#include "storage.hpp"
#include "util.hpp"
#include "random_walk_config.hpp"

using namespace std;

int main() {
    int threads = 2;
    cout << "threads" << endl;
    cin >> threads;
    
    Timer timer;
    // StdRandNumGenerator gen;
    #pragma omp parallel num_threads(threads)
    {
        walker_id_t walker_id = omp_get_thread_num();
        // debug
        // cout << walker_id << endl;
        StdRandNumGenerator gen;
        int num = 10000000 / threads;
        for (int i = 0; i < num; i++)
        {
            double a = gen.gen_float(1.0);
        }
    }
    cout << timer.duration() << endl;
    return 0;
}