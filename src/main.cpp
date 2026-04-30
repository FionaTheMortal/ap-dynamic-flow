#include "ap_dynamic_flow_parser.h"
#include "ap_dynamic_flow_graph.h"
#include "ap_dynamic_flow_update.h"
#include "ap_dynamic_flow_core.h"

#include <cstddef>
#include <algorithm>
#include <vector>
#include <queue>
#include <iostream>

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
    bool load_file    = false;

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

    bool initial_success = false;

    if (load_success)
    {
        s32 flow = run_push_relabel(&graph, graph.src_node, graph.dst_node);

        if (flow == graph.expected_max_flow)
        {
            std::cout << "Success!" << std::endl;

            initial_success = true;
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

    bool test_update = true;

    if (initial_success && test_update)
    {
        // NOTE: Dynamic update

        flow_graph graph_copy = graph;

        update_edge(&graph_copy, 2, 2, 2);

        // NOTE: Update and re-run

        flow_edge *edge = get_edge(&graph, 2, 2);

        edge->capacity = 2;

        s32 flow = run_push_relabel(&graph, graph.src_node, graph.dst_node);

        int i = 0;
    }

    return 0;
}
