#ifndef __GRAPH_H__
#define __GRAPH_H__

struct graph
{
    int num_edges;
    int num_nodes;

    // The node reached by vertex i's first outgoing edge is given by
    // outgoing_edges[outgoing_starts[i]].  To iterate over all
    // outgoing edges, please see the top-down bfs implementation.
    int* outgoing_starts;
    int* outgoing_edges;

    int* incoming_starts;
    int* incoming_edges;
};

void load_graph(const char* filename, graph* graph);
void load_graph_binary(const char* filename, graph* graph);
void store_graph_binary(const char* filename, graph* graph);

#endif
