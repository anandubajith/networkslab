#include <stdio.h>
#include "graph.h"

// need to implement priorityQ

void dijkstra(Graph *g, int start) {

    printf("\033[1m\033[37m");
    printf("\nLSR at node %d\n", start+1);
    printf("\033[0m");
    printf("+------+------+------+\n");
    printf("| dest | cost | path |\n");
    printf("+------+------+------+\n");
}

int main () {
    printf("Link State Routing => Dijkstra\n");
    Graph *g = input_graph();
    /*
     * Have global knowledge
     * Run Dijkstra
     */

    for ( int i = 0; i < g-> num_nodes; i++) {
        dijkstra(g, i);
    }

    print_graph(g);
    return 0;
}
