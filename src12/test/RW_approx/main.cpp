#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include <unordered_map>
#include <atomic>

#include "type.hpp"
#include "storage.hpp"
#include "util.hpp"
#include "param.hpp"
#include "ndcg.hpp"

using namespace std;

int main() {
    string str = "LiveJournal";
    string graph_path = "../../../graph_data/data/" + str + ".data";

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

    // 隣接リスト
    std::vector<std::vector<vertex_id_t>> adjacency_list(v_num);
    for (vertex_id_t v_i = 0; v_i < v_num; v_i++)
    {
        adjacency_list[v_i].reserve(vertex_degree[v_i]);
    }
    for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
    {
        adjacency_list[read_edges[e_i].src].push_back(read_edges[e_i].dst);
    }

    // 実験
    atomic<double> ndcg_sum;
    ndcg_sum = 0;
    cout << "start" << endl;

    #pragma omp parallel num_threads(THREADS_NUM)
    {
        StdRandNumGenerator gen;
        uint32_t ex_id = omp_get_thread_num();
        // double pprval_per_walker = 1.0 / RW_NUM;

        while (1) {
            if (ex_id >= START_VERTEX_NUM) break;

            // 開始頂点の決定
            vertex_id_t start_v = gen.gen(v_num);

            // 正しいPPR
            unordered_map<int, double> exact_ppr;
            {
                int all_path_length = 0;
                for (int i = 0; i < RW_NUM; i++) {
                    vertex_id_t v_i = start_v;
                    vector<int> path;

                    while (1) {
                        path.push_back(v_i);

                        if (vertex_degree[v_i] == 0 || gen.gen_float(1.0) < ALPHA) {
                            break;
                        } else {
                            vertex_id_t next_v = adjacency_list[v_i][gen.gen(vertex_degree[v_i])];
                            v_i = next_v;
                        }
                    }

                    for (int v : path) {
                        exact_ppr[v] += 1;
                        all_path_length += 1;
                    }
                }

                for (auto& [v, val] : exact_ppr) {
                    val /= (double)all_path_length;
                }
            }

            // 擬似パケロスを入れたときの PPR
            unordered_map<int, double> approx_ppr;
            {
                int all_path_length = 0;
                for (int i = 0; i < RW_NUM; i++) {
                    vertex_id_t v_i = start_v;
                    vector<int> path;
                    bool ok = false;

                    while (1) {
                        path.push_back(v_i);
                        
                        if (vertex_degree[v_i] == 0 || gen.gen_float(1.0) < ALPHA) {
                            ok = true;
                            break;
                        } else {
                            vertex_id_t next_v = adjacency_list[v_i][gen.gen(vertex_degree[v_i])];

                            if (v_i % 5 != 4 && next_v % 5 == 4) {
                                if (gen.gen_float(100.0) < DROP_PROB) break;
                            }

                            v_i = next_v;
                        }
                    }

                    if (ok) {
                        for (int v : path) {
                            approx_ppr[v] += 1;
                            all_path_length += 1;
                        }
                    }
                }

                for (auto& [v, val] : approx_ppr) {
                    val /= (double)all_path_length;
                }
            }

            double ndcg = calc_ndcg(exact_ppr, approx_ppr);
            // cout << ndcg << endl;

            ndcg_sum = ndcg_sum + ndcg;

            ex_id += THREADS_NUM;
            if (ex_id % 20 == 0) cout << ex_id << endl;
        }

    }

    double ave_ndcg = ndcg_sum / START_VERTEX_NUM;
    cout << ave_ndcg << endl;
    return 0;
}