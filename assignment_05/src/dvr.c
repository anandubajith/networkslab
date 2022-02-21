#include <stdio.h>
#include "graph.h"

int main (int argc, char *argv[])
{
    printf("Distance Vecctor Routing => Bellman Ford\n");
    Graph* g= input_graph();

    // do bellman ford here

    // print the routing tables for each router

    print_graph(g);
    return 0;
}
