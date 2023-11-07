#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <atomic>

#include "type.hpp"
#include "storage.hpp"
#include "util.hpp"
#include "random_walk_config.hpp"

using namespace std;

bool write_flag = false;

int main() {
    // string str;
    // cout << "filename" << endl;
    // cin >> str;
    string str = "LiveJournal";
    string graph_path = "../../../graph_data/data/" + str + ".data";

    // vertex_id_t v_num;
    // cout << "v_num" << endl;
    // cin >> v_num;
    vertex_id_t v_num = 1;

    // グラフ読み込み
    Edge<EmptyData> *read_edges; 
    edge_id_t read_e_num; 
    read_graph(graph_path.c_str(), read_edges, read_e_num);

    cout << read_e_num << endl;
    cout << read_edges[0].src << " " << read_edges[0].dst << endl;

    { // 無向グラフ変換
        Edge<EmptyData> *undirected_edges = new Edge<EmptyData>[read_e_num * 2];
        #pragma omp parallel for
        for (edge_id_t e_i = 0; e_i < read_e_num; e_i++)
        {
            undirected_edges[e_i * 2] = read_edges[e_i];
            std::swap(read_edges[e_i].src, read_edges[e_i].dst);
            undirected_edges[e_i * 2 + 1] = read_edges[e_i];
        }
        delete[] read_edges;
        read_edges = undirected_edges;
        read_e_num *= 2;
    }
    cout << "undirected " << read_e_num << endl;

    // 最大頂点IDを確認 (debug)
    vertex_id_t max_id = 0;
    for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
    {
        max_id = max(read_edges[e_i].src, max_id);
        max_id = max(read_edges[e_i].dst, max_id);
    }
    cout << max_id << endl;
    if (v_num-1 != max_id) v_num = max_id + 1;

    // 次数情報
    std::vector<vertex_id_t> vertex_degree(v_num, 0);
    for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
    {
        vertex_degree[read_edges[e_i].src]++;
    }

    // グラフのデータ保持方法を選択
    cout << "lil, map, csr, lil_pointer ?" << endl;
    string data_structure;
    cin >> data_structure;
    
    if (data_structure == "lil") // 隣接リスト
    {
        // 隣接リスト
        // Timer timer_data;

        std::vector<std::vector<vertex_id_t>> adjacency_list(v_num);
        for (vertex_id_t v_i = 0; v_i < v_num; v_i++)
        {
            adjacency_list[v_i].reserve(vertex_degree[v_i]);
        }
        for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
        {
            adjacency_list[read_edges[e_i].src].push_back(read_edges[e_i].dst);
        }

        // double t_d = timer_data.duration();
        // if (write_flag) {
        //     ofstream ofs;
        //     string f = "./output/data/createGraphTime_" + str + "_lil.txt";
        //     ofs.open(f, std::ios::app);
        //     ofs << t_d << endl;
        // }
        // cout << t_d << endl;


        // RW実行
        walker_id_t walker_num_per_vertex;
        cout << "walker_num_per_vertex" << endl;
        cin >> walker_num_per_vertex;
        walker_id_t walker_num_all = walker_num_per_vertex * v_num;

        uint16_t threads;
        cout << "threads" << endl;
        cin >> threads;

        uint16_t exe_num;
        cout << "exe_num" << endl;
        cin >> exe_num;

        for (int i = 0; i < exe_num; i++) 
        {
            RandomWalkConfig RW_config;
            // StdRandNumGenerator gen;
            atomic<int> end_count = 0;
            Timer timer;
            #pragma omp parallel num_threads(threads)
            {
                walker_id_t walker_id = omp_get_thread_num();
                // debug
                // cout << walker_id << endl;
                StdRandNumGenerator gen;

                while (1) 
                {
                    vertex_id_t v_i = walker_id % v_num;

                    while (1) {
                        if (vertex_degree[v_i] == 0 || gen.gen_float(1.0) < RW_config.getAlpha()) break;
                        else {
                            v_i = adjacency_list[v_i][gen.gen(vertex_degree[v_i])];
                        }
                    }

                    walker_id += threads;
                    end_count++;
                    if (walker_id >= walker_num_all) break;
                }

                // while (1) 
                // {
                //     vertex_id_t v_i = walker_id % v_num;

                //     uint16_t life = 1;
                //     while (gen.gen_float(1.0) > RW_config.getAlpha()) life++;

                //     while (life - 1) 
                //     {
                //         vertex_id_t degree = vertex_degree[v_i];
                //         if (degree == 0) break;
                //         v_i = adjacency_list[v_i][gen.gen(degree)];
                //         life--;
                //     }

                //     walker_id += threads;
                //     if (walker_id >= walker_num_all) break;
                // }
            }

            double t = timer.duration();
            if (write_flag)
            {
                ofstream ofs_time;
                string f = "./output/data/" + str + "_lil" + "_walkerNumPerVertex_" + to_string(walker_num_per_vertex) + "_threads_" + to_string(threads) + ".txt";
                ofs_time.open(f, std::ios::app);
                ofs_time << t << endl;
            }
            cout << t << " " << end_count << endl;
            
        }
    }
    else if (data_structure == "map") // unordered_map
    {
        // unordered_map
        // Timer timer_data;

        std::unordered_map<vertex_id_t, std::vector<vertex_id_t>> adjacency_list;
        for (vertex_id_t v_i = 0; v_i < v_num; v_i++)
        {
            adjacency_list[v_i].reserve(vertex_degree[v_i]);
        }
        for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
        {
            adjacency_list[read_edges[e_i].src].push_back(read_edges[e_i].dst);
        }

        // double t_d = timer_data.duration();
        // if (write_flag) {
        //     ofstream ofs;
        //     string f = "./output/data/createGraphTime_" + str + "_umap.txt";
        //     ofs.open(f, std::ios::app);
        //     ofs << t_d << endl;
        // }
        // cout << t_d << endl;

        // RW実行
        walker_id_t walker_num_per_vertex;
        cout << "walker_num_per_vertex" << endl;
        cin >> walker_num_per_vertex;
        walker_id_t walker_num_all = walker_num_per_vertex * v_num;

        uint16_t threads;
        cout << "threads" << endl;
        cin >> threads;

        uint16_t exe_num;
        cout << "exe_num" << endl;
        cin >> exe_num;

        for (int i = 0; i < exe_num; i++) 
        {

            RandomWalkConfig RW_config;
            // StdRandNumGenerator gen;
            Timer timer;
            #pragma omp parallel num_threads(threads)
            {
                walker_id_t walker_id = omp_get_thread_num();
                // debug
                // cout << walker_id << endl;
                StdRandNumGenerator gen;

                while (1) 
                {
                    vertex_id_t v_i = walker_id % v_num;

                    while (1) {
                        if (vertex_degree[v_i] == 0 || gen.gen_float(1.0) < RW_config.getAlpha()) break;
                        else {
                            v_i = adjacency_list[v_i][gen.gen(vertex_degree[v_i])];
                        }
                    }

                    walker_id += threads;
                    if (walker_id >= walker_num_all) break;
                }
            }

            double t = timer.duration();
            if (write_flag)
            {
                ofstream ofs_time;
                string f = "./output/data/" + str + "_umap" + "_walkerNumPerVertex_" + to_string(walker_num_per_vertex) + "_threads_" + to_string(threads) + ".txt";
                ofs_time.open(f, std::ios::app);
                ofs_time << t << endl;
            }
            cout << t << endl;
            
        }
    }
    else if (data_structure == "csr") // csr
    {
        // csr 作成
        // Timer timer_data;

        EdgeContainer<EmptyData> *csr = new EdgeContainer<EmptyData>();
        csr->adj_lists = new AdjList<EmptyData>[v_num];
        csr->adj_units = new AdjUnit<EmptyData>[read_e_num];
        edge_id_t chunk_edge_idx = 0;
        for (vertex_id_t v_i = 0; v_i < v_num; v_i++)
        {
            csr->adj_lists[v_i].begin = csr->adj_units + chunk_edge_idx;
            csr->adj_lists[v_i].end = csr->adj_lists[v_i].begin;
            chunk_edge_idx += vertex_degree[v_i];
        }
        for (edge_id_t e_i = 0; e_i < read_e_num; e_i++)
        {
            auto e = read_edges[e_i];
            auto ep = csr->adj_lists[e.src].end ++;
            ep->neighbour = e.dst;
        }

        // double t_d = timer_data.duration();
        // if (write_flag) {
        //     ofstream ofs;
        //     string f = "./output/data/createGraphTime_" + str + "_csr.txt";
        //     ofs.open(f, std::ios::app);
        //     ofs << t_d << endl;
        // }
        // cout << t_d << endl;

        // RW実行
        walker_id_t walker_num_per_vertex;
        cout << "walker_num_per_vertex" << endl;
        cin >> walker_num_per_vertex;
        walker_id_t walker_num_all = walker_num_per_vertex * v_num;

        uint16_t threads;
        cout << "threads" << endl;
        cin >> threads;

        uint16_t exe_num;
        cout << "exe_num" << endl;
        cin >> exe_num;

        for (int i = 0; i < exe_num; i++) 
        {

            RandomWalkConfig RW_config;
            // StdRandNumGenerator gen;
            Timer timer;
            #pragma omp parallel num_threads(threads)
            {
                walker_id_t walker_id = omp_get_thread_num();
                // debug
                // cout << walker_id << endl;
                StdRandNumGenerator gen;

                while (1) 
                {
                    vertex_id_t v_i = walker_id % v_num;

                    while (1) {
                        if (vertex_degree[v_i] == 0 || gen.gen_float(1.0) < RW_config.getAlpha()) break;
                        else {
                            AdjList<EmptyData> *adj = csr->adj_lists + v_i;
                            v_i = (adj->begin + gen.gen(vertex_degree[v_i]))->neighbour;
                        }
                    }

                    walker_id += threads;
                    if (walker_id >= walker_num_all) break;
                }
            }

            double t = timer.duration();
            if (write_flag)
            {
                ofstream ofs_time;
                string f = "./output/data/" + str + "_csr" + "_walkerNumPerVertex_" + to_string(walker_num_per_vertex) + "_threads_" + to_string(threads) + ".txt";
                ofs_time.open(f, std::ios::app);
                ofs_time << t << endl;
            }
            cout << t << endl;
            
        }

        delete csr;
    }
    else if (data_structure == "lil_pointer")
    {
        // 隣接リスト (vector ではなくポインタを使う)
        // Timer timer_data;

        vertex_id_t** adjacency_list = new vertex_id_t*[v_num];
        for (vertex_id_t v_i = 0; v_i < v_num; v_i++)
        {
            adjacency_list[v_i] = new vertex_id_t[vertex_degree[v_i]];
        }
        vector<int> idx(v_num, 0);
        for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
        {
            adjacency_list[read_edges[e_i].src][idx[read_edges[e_i].src]++] = read_edges[e_i].dst;
        }

        // double t_d = timer_data.duration();
        // if (write_flag) {
        //     ofstream ofs;
        //     string f = "./output/data/createGraphTime_" + str + "_lilPointer.txt";
        //     ofs.open(f, std::ios::app);
        //     ofs << t_d << endl;
        // }
        // cout << t_d << endl;

        // RW実行
        walker_id_t walker_num_per_vertex;
        cout << "walker_num_per_vertex" << endl;
        cin >> walker_num_per_vertex;
        walker_id_t walker_num_all = walker_num_per_vertex * v_num;

        uint16_t threads;
        cout << "threads" << endl;
        cin >> threads;

        uint16_t exe_num;
        cout << "exe_num" << endl;
        cin >> exe_num;

        for (int i = 0; i < exe_num; i++) 
        {

            RandomWalkConfig RW_config;
            // StdRandNumGenerator gen;
            Timer timer;
            #pragma omp parallel num_threads(threads)
            {
                walker_id_t walker_id = omp_get_thread_num();
                // debug
                // cout << walker_id << endl;
                StdRandNumGenerator gen;

                while (1) 
                {
                    vertex_id_t v_i = walker_id % v_num;

                    while (1) {
                        if (vertex_degree[v_i] == 0 || gen.gen_float(1.0) < RW_config.getAlpha()) break;
                        else {
                            v_i = adjacency_list[v_i][gen.gen(vertex_degree[v_i])];
                        }
                    }

                    walker_id += threads;
                    if (walker_id >= walker_num_all) break;
                }
            }

            double t = timer.duration();
            if (write_flag)
            {
                ofstream ofs_time;
                string f = "./output/data/" + str + "_lilPointer" + "_walkerNumPerVertex_" + to_string(walker_num_per_vertex) + "_threads_" + to_string(threads) + ".txt";
                // string f = "./output/data/" + str + "_lilPointer" + "_walkerNumPerVertex_" + to_string(walker_num_per_vertex) + "_threads_" + to_string(threads) + "_sameRandGen" + ".txt";
                ofs_time.open(f, std::ios::app);
                ofs_time << t << endl;
            }
            cout << t << endl;
        }
    }
    else // エラー
    {
        cout << "invalid data structure" << endl;
    }
    return 0;
}