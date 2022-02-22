#include <stdio.h>
#include "graph.h"


// need to implement priorityQ
//




void dijkstra(Graph *g, int start) {

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


    /* print_graph(g); */
    return 0;
}
