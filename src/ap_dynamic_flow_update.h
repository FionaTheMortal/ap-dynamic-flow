#ifndef AP_DYNAMIC_FLOW_UPDATE_H
#define AP_DYNAMIC_FLOW_UPDATE_H

void
update_relabel(flow_graph *graph, flow_node *node)
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
update_reset_node_data(flow_graph *graph)
{
    clear_active_node_queue(graph);

    for (flow_node &node : graph->nodes)
    {
        node.next_edge_to_push = 0;

        node.excess = 0;
        node.height = 0;
    }
}

void
update_push_along_edge(flow_graph *graph, flow_node *node, flow_edge *edge)
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

        if (dst_was_inactive && dst->excess > 0)
        {
            add_active_node(graph, dst);
        }
    }
}

void
update_discharge(flow_graph *graph, flow_node *node)
{
    while (node->excess > 0)
    {
        if (node->next_edge_to_push < node->outgoing_edges.size())
        {
            flow_edge *edge = get_edge(node, node->next_edge_to_push);

            if (can_push_along_edge(graph, node, edge))
            {
                update_push_along_edge(graph, node, edge);
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

void
update_edge(flow_graph *graph, s32 node_index, s32 edge_index, s32 new_capacity)
{
    flow_edge *edge = get_edge(graph, node_index, edge_index);

    s32 old_capacity = edge->capacity;

    if (new_capacity != old_capacity)
    {
        flow_node *src = get_node(graph, node_index);
        flow_node *dst = get_node(graph, edge->dst_node);

        if (new_capacity > old_capacity)
        {
            s32 capacity_delta = new_capacity - old_capacity;

            // NOTE: Case 1: Increase in capacity

            /* 
             * NOTE: Algorithm idea:
             * The flow is only affected if edge is on the border of all min cuts
             * In this case graph src can reach edge src and edge dst can reach sink
             * The BFSs can be terminated if either finds the node on the other side of the edge -> Not a min cut
             * Run regular algorithm on network of edges that are reachable
             * 
             */

            if (get_remaining_capacity(edge) == 0)
            {
                TODO_IMPLEMENT;
            }
            else
            {
                // NOTE: If the edge wasn't saturated it can't separate min cuts -> done.
            }
        }
        else
        {
            // NOTE: Case 2: Decrease in capacity

            s32 capacity_delta = old_capacity - new_capacity;

            if (edge->flow > new_capacity)
            {
                // NOTE: Flow is affected -> work to do

                /*  
                 * NOTE: Algorithm idea:
                 *  - Add overflow as excess to edge_src 
                 *  - Subtract overflow as excess from edge_dst
                 *  - Add a new graph source node that connects to the original source.
                 *      - This allows the algorithm to terminate, meaning if edge dst is unreachable the overflow
                 *        can escape using the new source
                 *      - This keeps the overall flow information in the graph intakt
                 *      - I think one could short cut the computation if preserving the flow network isn't necessary
                 *  - Flow that couldn't reach edge_dst is transported to new source -> easy to determine the flow delta
                 */

                s32 flow_delta = edge->flow - new_capacity;

                update_reset_node_data(graph);

                edge->capacity = new_capacity;
                edge->flow     = new_capacity;
                
                flow_edge *reverse = get_reverse_edge(graph, edge);

                reverse->flow = -edge->flow;

                umi new_source_index = graph->nodes.size();
                
                add_node(graph);
                add_edge_pair(graph, graph->src_node, new_source_index, flow_delta);

                flow_node *new_source = get_node(graph, new_source_index);

                new_source->excess = -flow_delta;
                new_source->height = (s32)graph->nodes.size();

                flow_node *src = get_node(graph, node_index);
                flow_node *dst = get_node(graph, edge->dst_node);

                src->excess += flow_delta;
                dst->excess -= flow_delta;

                add_active_node(graph, src);

                while (!graph->active_nodes.empty())
                {
                    flow_node *node = pop_active_node(graph);

                    if (node != new_source)
                    {
                        update_discharge(graph, node);
                    }
                }

                int i = 0;
            }
            else
            {
                // NOTE: Flow not affected -> Done.
            }
        }
    }
    else
    {
        TODO_IMPLEMENT;
    }

    int i = 0;
}

#endif
