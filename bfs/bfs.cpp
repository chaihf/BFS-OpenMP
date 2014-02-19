#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>
#include <omp.h>

#include "CycleTimer.h"
#include "bfs.h"
#include "graph.h"

#define ROOT_NODE_ID                    0
#define NOT_VISITED_MARKER              -1
#define BOTTOMUP_NOT_VISITED_MARKER     0
#define NUM_THREADS                     12 //GHC3000 6 cores with hyperthreading




void vertex_set_clear(vertex_set* list) {
    list->count = 0;
}

void vertex_set_init(vertex_set* list, int count) {
    list->alloc_count = count;
    list->present = (int*)malloc(sizeof(int) * list->alloc_count);
    vertex_set_clear(list);
}



void bottom_up_step(
    graph* g,
    vertex_set* frontier,    
    int* distances,
    int iteration) {

    int local_count = 0;
    // #pragma omp parallel num_threads(NUM_THREADS) private(local_count) 
    // #pragma omp parallel private(local_count)
    #pragma omp parallel 
    {
        // local_count = 0;
        #pragma omp for reduction(+: local_count)
        for (int i = 0; i < g->num_nodes; i++) {
            if (frontier->present[i] == BOTTOMUP_NOT_VISITED_MARKER) {
                int start_edge = g->incoming_starts[i];
                int end_edge = (i == g->num_nodes-1)? g->num_edges : g->incoming_starts[i + 1];
                for(int neighbor = start_edge; neighbor < end_edge; neighbor++) {
                    int incoming = g->incoming_edges[neighbor];
                    // if(__sync_bool_compare_and_swap(&frontier->present[incoming], iteration, distances[node] + 1))
                    if(frontier->present[incoming] == iteration) {
                        distances[i] = distances[incoming] + 1;
                        // frontier->count++;
                        local_count++;
                        frontier->present[i] = iteration + 1;
                        break;
                    }
                }
            }
        }

    }
    frontier->count = local_count;

}

void bfs_bottom_up(graph* graph, solution* sol)
{
    // 15-418/618 students:
    //
    // You will need to implement the "bottom up" BFS here as
    // described in the handout.
    //
    // As a result of your code's execution, sol.distances should be
    // correctly populated for all nodes in the graph.
    //
    // As was done in the top-down case, you may wish to organize your
    // code by creating subroutine bottom_up_step() that is called in
    // each step of the BFS process.

    vertex_set list1;
    
    vertex_set_init(&list1, graph->num_nodes);
    
    int iteration = 1;

    vertex_set* frontier = &list1;    
        
    // setup frontier with the root node    
    // just like put the root into queue
    frontier->present[frontier->count++] = 1;

    // set the root distance with 0
    sol->distances[ROOT_NODE_ID] = 0;
    
    // just like pop the queue
    while (frontier->count != 0) {
        
        frontier->count = 0;
        double start_time = CycleTimer::currentSeconds();
        

        bottom_up_step(graph, frontier, sol->distances, iteration);

        double end_time = CycleTimer::currentSeconds();
        printf("frontier=%-10d %.4f sec\n", frontier->count, end_time - start_time);

        iteration++;

    }



}




// Take one step of "top-down" BFS.  For each vertex on the frontier,
// follow all outgoing edges, and add all neighboring vertices to the
// new_frontier.
void top_down_step(
    graph* g,
    vertex_set* frontier,
    vertex_set* new_frontier,
    int* distances)
{
    int local_count;

    // #pragma omp parallel num_threads(NUM_THREADS) private(local_count) 
     #pragma omp parallel private(local_count)
    {
        local_count = 0;
        int* local_frontier = (int*)malloc(sizeof(int) * (g->num_nodes/NUM_THREADS));
        #pragma omp for 
        for (int i=0; i<frontier->count; i++) {            

            int node = frontier->present[i];

            int start_edge = g->outgoing_starts[node];
            int end_edge = (node == g->num_nodes-1) ? g->num_edges : g->outgoing_starts[node+1];

            // attempt to add all neighbors to the new frontier
            for (int neighbor=start_edge; neighbor<end_edge; neighbor++) {
                int outgoing = g->outgoing_edges[neighbor];

                if( __sync_bool_compare_and_swap(&distances[outgoing], NOT_VISITED_MARKER, distances[node] + 1)) {                    
                    local_frontier[local_count] = outgoing;
                    local_count++;
                }
            }
        }

        #pragma omp critical                    
        {
            memcpy(new_frontier->present+new_frontier->count, local_frontier, local_count*sizeof(int));
            new_frontier->count += local_count;
        }
    }
}

// Implements top-down BFS.
//
// Result of execution is that, for each node in the graph, the
// distance to the root is stored in sol.distances.
void bfs_top_down(graph* graph, solution* sol) {

    vertex_set list1;
    vertex_set list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);

    vertex_set* frontier = &list1;
    vertex_set* new_frontier = &list2;

    // initialize all nodes to NOT_VISITED
    for (int i=0; i<graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;
    
        
    // setup frontier with the root node    
    // just like put the root into queue
    frontier->present[frontier->count++] = ROOT_NODE_ID;

    // set the root distance with 0
    sol->distances[ROOT_NODE_ID] = 0;

    // just like pop the queue
    while (frontier->count != 0) {

        double start_time = CycleTimer::currentSeconds();

        vertex_set_clear(new_frontier);

        top_down_step(graph, frontier, new_frontier, sol->distances);

        double end_time = CycleTimer::currentSeconds();
        printf("frontier=%-10d %.4f sec\n", frontier->count, end_time - start_time);

        // swap pointers
        vertex_set* tmp = frontier;
        frontier = new_frontier;
        new_frontier = tmp;
    }
}
