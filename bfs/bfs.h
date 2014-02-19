#ifndef __BFS_H__
#define __BFS_H__

struct graph;

struct solution
{
    int* distances;
};

struct vertex_set {
    int  count;
    int  alloc_count;
    int* present;
};


void bfs_bottom_up(graph* graph, solution* sol);
void bfs_top_down(graph* graph, solution* sol);

#endif
