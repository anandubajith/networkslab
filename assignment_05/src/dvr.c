#include <stdio.h>
#include<limits.h>
#include "graph.h"

int main (int argc, char *argv[])
{
    printf("Distance Vecctor Routing => Bellman Ford\n");
    Graph* g= input_graph();

    // do bellman ford here

    int dist[g->num_nodes];
    for ( int i = 0; i < g->num_nodes; g++) {
        dist[i] = INT_MAX;
    }
    dist[0] = 0;



    // print the routing tables for each router

    print_graph(g);
    return 0;
}
