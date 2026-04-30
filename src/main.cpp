#include "ap_dynamic_flow_core.h"
#include "ap_dynamic_flow_parser.h"

#include <cstddef>
#include <cassert>
#include <algorithm>
#include <vector>
#include <queue>
#include <iostream>

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

s32
run_push_relabel(flow_graph *graph, umi src, umi dst)
{
    assert(graph->nodes.size() <= MAX_NODE_COUNT);

    init_temp_data(graph);

    flow_node *src_node = get_node(graph, src);
    flow_node *dst_node = get_node(graph, dst);

    src_node->excess = INF_EXCESS;
    src_node->height = (s32)graph->nodes.size();

    for (flow_edge &edge : src_node->outgoing_edges)
    {
        push_along_edge(graph, src_node, &edge);
    }

    while (!graph->active_nodes.empty())
    {
        flow_node *node = pop_active_node(graph);

        if (node != src_node && node != dst_node)
        {
            discharge(graph, node);
        }
    }

    s32 max_flow = 0;

    for (flow_edge &edge : src_node->outgoing_edges)
    {
        max_flow += edge.flow;
    }

    return max_flow;
}

// TODO: Lower or increase the capacity along an edge

s32
get_load_graph_node_count(loaded_graph *graph)
{
    s32 result = 0;

    for (loaded_edge &edge : graph->edges)
    {
        s32 edge_max_node_index = std::max(edge.src_node, edge.dst_node);
        s32 edge_min_node_count = edge_max_node_index + 1;

        result = std::max(result, edge_min_node_count);
    }

    return result;
}

void
translate_loaded_graph_to_flow_graph(loaded_graph *loaded, flow_graph *flow)
{
    s32 node_count = get_load_graph_node_count(loaded);

    for (s32 index = 0;
        index < node_count;
        ++index)
    {
        add_node(flow);   
    }

    for (loaded_edge &edge : loaded->edges)
    {
        add_edge_pair(flow, edge.src_node, edge.dst_node, edge.capacity);
    }

    flow->dst_node = loaded->dst_node;
    flow->src_node = loaded->src_node;

    flow->expected_max_flow = loaded->max_flow;
}

bool
load_graph_from_dir(flow_graph *graph, std::string_view dir)
{
    bool success = false;

    loaded_graph loaded = load_graph(dir);

    if (loaded.is_loaded)
    {
        translate_loaded_graph_to_flow_graph(&loaded, graph);

        success = true;
    }

    return success;
}

void
init_test_graph_directly(flow_graph *graph)
{
    // NOTE: Copied the graph from wikipedia C referrence implementation

    s32 node_count = 6;

    for (s32 index = 0;
        index < node_count;
        ++index)
    {
        add_node(graph);   
    }

    add_edge_pair(graph, 0, 1, 2);
    add_edge_pair(graph, 0, 2, 9);
    add_edge_pair(graph, 1, 2, 1);
    add_edge_pair(graph, 1, 3, 0);
    add_edge_pair(graph, 1, 4, 0);
    add_edge_pair(graph, 2, 4, 7);
    add_edge_pair(graph, 3, 5, 7);
    add_edge_pair(graph, 4, 5, 4);

    graph->src_node = 0;
    graph->dst_node = 5;
    graph->expected_max_flow = 4;
}

int
main()
{
    flow_graph graph;

    bool load_success = false;
    bool load_file = true;

    if (load_file)
    {
        if (load_graph_from_dir(&graph, "graphs/graph_000001"))
        {
            load_success = true;
        }
    }
    else
    {
        init_test_graph_directly(&graph);

        load_success = true;
    }

    if (load_success)
    {
        s32 flow = run_push_relabel(&graph, graph.src_node, graph.dst_node);

        if (flow == graph.expected_max_flow)
        {
            std::cout << "Success!" << std::endl;
        }
        else
        {
            std::cout << "Calculated max flow doesn't match expected value." << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to load graph." << std::endl;
    }

    return 0;
}
