#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string>
#include <getopt.h>

#include "CycleTimer.h"
#include "graph.h"
#include "bfs.h"


#define USE_BINARY_GRAPH 1


void usage(const char* progname) {
    printf("Usage: %s [options] graph_file_name\n", progname);
    printf("Program Options:\n");
    printf("  -t  --threads <N>  Use T threads\n");
    printf("  -?  --help         This message\n");
}

int main(int argc, char** argv) {

    int  num_threads = -1;
    std::string graph_filename;

    // parse commandline options ////////////////////////////////////////////
    int opt;
    static struct option long_options[] = {
        {"threads", 1, 0, 't'},
        {"help", 0, 0, '?'},
        {0 ,0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "t:?", long_options, NULL)) != EOF) {

        switch (opt) {
        case 't':
        {
            num_threads = atoi(optarg);
            break;
        }
        case '?':
        default:
            usage(argv[0]);
            return 1;
        }
    }
    // end parsing of commandline options


    if (optind >= argc) {
        usage(argv[0]);
        return 1;
    }

    graph_filename = argv[optind];

    graph g;

    printf("----------------------------------------------------------\n");
    printf("OMP Max system threads = %d\n", omp_get_max_threads());
    if (num_threads > 0)
        omp_set_num_threads(num_threads);
    printf("OMP will use at most %d threads.\n", omp_get_max_threads());
    printf("----------------------------------------------------------\n");

    printf("Loading graph (this can take some time for the bigger graphs)...\n");
    load_graph_binary(graph_filename.c_str(), &g);

    printf("Graph stats:\n");
    printf("  Edges: %d\n", g.num_edges);
    printf("  Nodes: %d\n", g.num_nodes);

    solution sol1;
    sol1.distances = (int*)malloc(sizeof(int) * g.num_nodes);
    solution sol2;
    sol2.distances = (int*)malloc(sizeof(int) * g.num_nodes);
    solution sol3;
    sol3.distances = (int*)malloc(sizeof(int) * g.num_nodes);

    // execute bottom-up version

    start_time = CycleTimer::currentSeconds();
    bfs_bottom_up(&g, &sol1);
    end_time = CycleTimer::currentSeconds();
    printf("Bottom up BFS time: %.3f sec\n", end_time - start_time);

    // execute top-down version

    double start_time = CycleTimer::currentSeconds();
    bfs_top_down(&g, &sol2);
    double end_time = CycleTimer::currentSeconds();
    printf("Top down BFS time: %.3f sec\n", end_time - start_time);

    // execute hybrid version

    start_time = CycleTimer::currentSeconds();
    bfs_hybrid(&g, &sol3);
    end_time = CycleTimer::currentSeconds();
    printf("Hybrid BFS time: %.3f sec\n", end_time - start_time);


    for (int i=0; i<g.num_nodes; i++) {
        if (sol1.distances[i] != sol2.distances[i]) {
            fprintf(stderr, "*** Distance results disagree at node %d: %d, %d\n", i, sol1.distances[i], sol2.distances[i]);
            exit(1);
        }
    }

    printf("Bottom-up and top-down distance results agree.\n");

    return 0;
}
