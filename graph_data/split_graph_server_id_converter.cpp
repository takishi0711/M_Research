#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#include "type.hpp"
#include "storage.hpp"

using namespace std;

int main() {
    string filename = "LiveJournal";
    int split_num = 7;
    string input_path = "./source_data/" + filename + ".txt";

    // サーバー情報読み取り
    vector<string> server_id;
    FILE *f = fopen("server.txt", "r");
    assert(f != NULL);
    char ch[100];
    while (1 == fscanf(f, "%s", &ch))
    {
        string str(ch);
        server_id.push_back(str);
    }
    fclose(f);
    // for (auto id : server_id) {
    //     cout << id << endl;
    // }

    // source data 読み取り
    vector<vector<Edge_dstIp>> edges(split_num);
    FILE *in_f = fopen(input_path.c_str(), "r");
    assert(in_f != NULL);
    vertex_id_t src, dst;
    while (2 == fscanf(in_f, "%u %u", &src, &dst))
    {
        edges[src%split_num].push_back(Edge_dstIp(src, dst, dst%split_num));
        edges[dst%split_num].push_back(Edge_dstIp(dst, src, src%split_num));
    }
    fclose(in_f);

    for (int i = 0; i < split_num; i++) {
        auto es = edges[i].data();
        auto e_num = edges[i].size();
        string output_path = "./" + filename + "/" + to_string(split_num) + "/" + server_id[i] + ".data";
        FILE *out_f = fopen(output_path.c_str(), "w");
        assert(out_f != NULL);
        auto ret = fwrite(es, sizeof(Edge_dstIp), e_num, out_f);
        assert(ret == e_num);
        fclose(out_f);
    }

    // test
    Edge_dstIp *read_edges;
    edge_id_t read_e_num;
    read_graph("./LiveJournal/7/10.58.60.1.data", read_edges, read_e_num);

    FILE *test_f = fopen("test2.txt", "w");
    for (int i = 0; i < read_e_num; i++) {
        fprintf(test_f, "%d %d %d\n", read_edges[i].src, read_edges[i].dst, read_edges[i].dst_ip);
    }
    fclose(test_f);
    return 0;
}