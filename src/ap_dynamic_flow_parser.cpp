#include "ap_dynamic_flow_parser.h"

#include <nlohmann/json.hpp>
#include <csv/csv.h>

#include <string>
#include <vector>
#include <string_view>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

using json = nlohmann::json;

void
add_edge(loaded_graph *graph, s32 src_node, s32 dst_node, s32 capacity)
{
    loaded_edge edge;
    edge.capacity = capacity;
    edge.dst_node = dst_node;
    edge.src_node = src_node;

    graph->edges.push_back(edge);
}

bool
load_graph_meta_json(loaded_graph *graph, fs::path path)
{
    bool success = false;

    std::ifstream stream(path);

    if (stream)
    {
        try 
        {
            json data = json::parse(stream);

            graph->src_node = data["s"];
            graph->dst_node = data["t"];

            success = true;
        }
        catch (...) 
        {
            success = false;
        }
    }

    return success;
}

bool
load_graph_edges_csv(loaded_graph *graph, fs::path path, bool has_header = true)
{
    bool success = false;

    try 
    {
        io::CSVReader<3> csv_reader(path.string());

        if (has_header) 
        {
            std::string c1, c2, c3;
            csv_reader.read_row(c1, c2, c3);
        }

        s32 src_node = 0;
        s32 dst_node = 0;
        s32 capacity = 0;

        while (csv_reader.read_row(src_node, dst_node, capacity)) 
        {
            add_edge(graph, src_node, dst_node, capacity);
        }

        success = true;
    }
    catch (...) 
    {
        success = false;
    }

    return success;
}

bool
load_graph_cut_json(loaded_graph *graph, fs::path path)
{
    bool success = false;

    std::ifstream stream(path);

    if (stream)
    {
        try 
        {
            json data = json::parse(stream);

            graph->max_flow = data["maxflow"];

            success = true;
        }
        catch (...) 
        {
            success = false;
        }
    }

    return success;
}

loaded_graph 
load_graph(std::string_view dir_path)
{
    loaded_graph graph;
    graph.dir_path = dir_path;

    fs::path directory(dir_path);
    fs::path meta_json = directory / "meta.json";

    if (load_graph_meta_json(&graph, meta_json))
    {
        fs::path edges_csv = directory / "edges.csv";

        if (load_graph_edges_csv(&graph, edges_csv))
        {
            fs::path cut_json  = directory / "cut.json";

            if (load_graph_cut_json(&graph, cut_json))
            {
                graph.is_loaded = true;
            }
        }
    }

    return graph;
}
