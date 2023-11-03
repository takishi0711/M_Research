#pragma once

#include <stdint.h>
#include <utility>

typedef uint32_t vertex_id_t;
typedef uint64_t edge_id_t;

struct EmptyData
{
};

template <typename edge_data_t>
struct Edge
{
    vertex_id_t src;
    vertex_id_t dst;
    edge_data_t data;
    Edge() {}
    Edge(vertex_id_t _src, vertex_id_t _dst, edge_data_t _data) : src(_src), dst(_dst), data(_data) {}
    bool friend operator == (const Edge<edge_data_t> &a, const Edge<edge_data_t> &b)
    {
        return (a.src == b.src
            && a.dst == b.dst
            && a.data == b.data
        );
    }
    void transpose()
    {
        std::swap(src, dst);
    }
};

template <>
struct Edge <EmptyData>
{
    vertex_id_t src;
    union
    {
        vertex_id_t dst;
        EmptyData data;
    };
    Edge() {}
    Edge(vertex_id_t _src, vertex_id_t _dst) : src(_src), dst(_dst) {}
    bool friend operator == (const Edge<EmptyData> &a, const Edge<EmptyData> &b)
    {
        return (a.src == b.src
            && a.dst == b.dst
        );
    }
    void transpose()
    {
        std::swap(src, dst);
    }
};