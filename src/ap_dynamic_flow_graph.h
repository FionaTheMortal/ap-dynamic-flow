#ifndef AP_DYNAMIC_FLOW_GRAPH_H
#define AP_DYNAMIC_FLOW_GRAPH_H

#include "ap_dynamic_flow_core.h"

#include <algorithm>
#include <vector>
#include <queue>

// TODO: Check if these are actually right

constexpr s32 MAX_HEIGHT     = INT32_MAX; 
constexpr s32 MAX_NODE_COUNT = MAX_HEIGHT / 2;
constexpr s32 INF_EXCESS     = INT32_MAX;

struct flow_edge
{
    umi dst_node     = 0;
    umi reverse_edge = 0;

    s32 capacity = 0;
    s32 flow     = 0;
};

struct flow_node
{
    std::vector<flow_edge> outgoing_edges;

    umi next_edge_to_push = 0;

    s32 excess = 0;
    s32 height = 0;
};

struct flow_graph
{
    std::vector<flow_node> nodes;

    std::queue<flow_node *> active_nodes;

    s32 src_node = 0;
    s32 dst_node = 0;

    s32 expected_max_flow = 0;
};

flow_node *
get_node(flow_graph *graph, umi index)
{
    flow_node *result = nullptr;

    if (index < graph->nodes.size())
    {
        result = &graph->nodes[index];
    }

    return result;
}

flow_edge *
get_edge(flow_node *node, umi index)
{
    flow_edge *result = nullptr;

    if (index < node->outgoing_edges.size())
    {
        result = &node->outgoing_edges[index];
    }

    return result;
}

flow_edge *
get_edge(flow_graph *graph, umi node_index, umi edge_index)
{
    flow_node *node = get_node(graph, node_index);

    flow_edge *result = get_edge(node, edge_index);

    return result;
}

s32
get_remaining_capacity(flow_edge *edge)
{
    assert(edge->flow <= edge->capacity);

    s32 result = edge->capacity - edge->flow;

    return result;
}

void
add_edge_pair(flow_graph *graph, umi src, umi dst, s32 capacity)
{
    flow_node *src_node = get_node(graph, src);
    flow_node *dst_node = get_node(graph, dst);

    assert(src_node != nullptr);
    assert(dst_node != nullptr);

    umi edge_to_dst_index = src_node->outgoing_edges.size();
    umi edge_to_src_index = dst_node->outgoing_edges.size();

    flow_edge edge_to_dst;
    edge_to_dst.dst_node = dst;
    edge_to_dst.capacity = capacity;
    edge_to_dst.reverse_edge = edge_to_src_index;

    flow_edge edge_to_src;
    edge_to_src.dst_node = src;
    edge_to_src.capacity = 0;
    edge_to_src.reverse_edge = edge_to_dst_index;

    src_node->outgoing_edges.push_back(edge_to_dst);
    dst_node->outgoing_edges.push_back(edge_to_src);
}

void
add_node(flow_graph *graph)
{
    flow_node node;

    graph->nodes.push_back(node);
}

flow_edge *
get_reverse_edge(flow_graph *graph, flow_edge *edge)
{
    flow_node *dst_node = get_node(graph, edge->dst_node);
    
    flow_edge *result = get_edge(dst_node, edge->reverse_edge);

    return result;
}

void
clear_active_node_queue(flow_graph *graph)
{
    std::queue<flow_node *> empty;
    std::swap(graph->active_nodes, empty);
}

void
init_temp_data(flow_graph *graph)
{
    clear_active_node_queue(graph);

    for (flow_node &node : graph->nodes)
    {
        node.next_edge_to_push = 0;

        node.excess = 0;
        node.height = 0;

        for (flow_edge &edge : node.outgoing_edges)
        {
            edge.flow = 0;
        }
    }
}

void
add_active_node(flow_graph *graph, flow_node *node)
{
    graph->active_nodes.push(node);
}

flow_node *
pop_active_node(flow_graph *graph)
{
    flow_node *result = graph->active_nodes.front();

    graph->active_nodes.pop();

    return result;
}

void
push_along_edge(flow_graph *graph, flow_node *node, flow_edge *edge)
{
    flow_node *src = node;
    flow_node *dst = get_node(graph, edge->dst_node);

    flow_edge *edge_from_src = edge;
    flow_edge *edge_from_dst = get_reverse_edge(graph, edge);

    int to_push = std::min(src->excess, get_remaining_capacity(edge));

    if (to_push > 0)
    {
        bool dst_was_inactive = (dst->excess == 0);
        
        src->excess -= to_push;
        dst->excess += to_push;

        edge_from_src->flow += to_push;
        edge_from_dst->flow -= to_push;

        if (dst_was_inactive)
        {
            add_active_node(graph, dst);
        }
    }
}

bool
can_push_along_edge(flow_graph *graph, flow_node *node, flow_edge *edge)
{
    flow_node *src = node;
    flow_node *dst = get_node(graph, edge->dst_node);

    bool result = false;

    if (get_remaining_capacity(edge) > 0 && src->height > dst->height)
    {
        result = true;
    }

    return result;
}

void
relabel(flow_graph *graph, flow_node *node)
{
    s32 lowest_neighbor = MAX_HEIGHT;

    for (flow_edge &edge : node->outgoing_edges)
    {
        if (get_remaining_capacity(&edge))
        {
            flow_node *node = get_node(graph, edge.dst_node);

            lowest_neighbor = std::min(lowest_neighbor, node->height);
        }
    }

    if (lowest_neighbor < MAX_HEIGHT)
    {
        node->height = lowest_neighbor + 1;
    }
}

void
discharge(flow_graph *graph, flow_node *node)
{
    while (node->excess > 0)
    {
        if (node->next_edge_to_push < node->outgoing_edges.size())
        {
            flow_edge *edge = get_edge(node, node->next_edge_to_push);

            if (can_push_along_edge(graph, node, edge))
            {
                push_along_edge(graph, node, edge);
            }

            ++node->next_edge_to_push;
        }
        else
        {
            relabel(graph, node);

            node->next_edge_to_push = 0;
        }
    }
}

#endif
