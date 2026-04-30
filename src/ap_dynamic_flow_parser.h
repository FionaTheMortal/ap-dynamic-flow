#ifndef AP_DYNAMIC_FLOW_PARSER_H
#define AP_DYNAMIC_FLOW_PARSER_H

#include "ap_dynamic_flow_core.h"

#include <string>
#include <vector>
#include <string_view>

struct loaded_edge
{
    s32 src_node = 0;
    s32 dst_node = 0;
    s32 capacity = 0;
};

struct loaded_graph
{
    bool is_loaded = false;

    std::string dir_path;

    std::vector<loaded_edge> edges;

    s32 src_node = 0;
    s32 dst_node = 0;
    s32 max_flow = 0;
};

loaded_graph load_graph(std::string_view dir_path);

#endif
