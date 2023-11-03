#include <iostream>
#include <string>
#include <vector>

#include "type.hpp"
#include "storage.hpp"

using namespace std;

int main() {
    string str;
    cout << "filename" << endl;
    cin >> str;
    string graph_path = "../../../graph_data/data/" + str + ".data";

    vertex_id_t v_num;
    cout << "v_num" << endl;
    cin >> v_num;

    Edge<EmptyData> *read_edges; 
    edge_id_t read_e_num; 
    read_graph(graph_path.c_str(), read_edges, read_e_num);

    cout << read_e_num << endl;
    cout << read_edges[0].src << " " << read_edges[0].dst << endl;

    {
        Edge<EmptyData> *undirected_edges = new Edge<EmptyData>[read_e_num * 2];
        #pragma omp parallel for
        for (edge_id_t e_i = 0; e_i < read_e_num; e_i++)
        {
            undirected_edges[e_i * 2] = read_edges[e_i];
            std::swap(read_edges[e_i].src, read_edges[e_i].dst);
            undirected_edges[e_i * 2 + 1] = read_edges[e_i];
        }
        delete []read_edges;
        read_edges = undirected_edges;
        read_e_num *= 2;
    }

    std::vector<vertex_id_t> vertex_degree(v_num, 0);
    for (edge_id_t e_i = 0; e_i < read_e_num; e_i++) 
    {
        vertex_degree[read_edges[e_i].src]++;
    }

    
    return 0;
}