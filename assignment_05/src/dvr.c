#include <stdio.h>
#include<limits.h>
#include "graph.h"

void bellman_ford(Graph *g, int start){
    int dist[g->num_nodes];
    int next_hop[g->num_nodes];
    for ( int i = 0; i < g->num_nodes; i++) {
        dist[i] = INT_MAX;
        next_hop[i] = INT_MAX;
    }
    dist[start] = 0;
    next_hop[start] = start;

    for ( int i = 0; i < g->num_nodes; i++) {
        for ( int j = 0; j < g->num_edges; j++) {
            Edge e = g->edges[j];
            if ( dist[e.from] != INT_MAX && dist[e.from] + e.cost < dist[e.to]) {
                dist[e.to] = dist[e.from] + e.cost;
                // handling special case [ TODO  this needs to be checked ]
                if ( e.from == start) {
                    next_hop[e.to] = e.to;
                } else {
                    next_hop[e.to] = e.from;
                }
            }
        }
    }
    // todo check for negative weight cycle


    // print the routing tables for each router
    printf("\033[1m\033[37m");
    printf("\nRouting Table at node %d\n", start+1);
    printf("\033[0m");
    printf("+------+------+------+\n");
    printf("| dest | next | cost |\n");
    printf("+------+------+------+\n");
    for ( int i = 0; i < g->num_nodes; i++) {
        printf("| %-4d |", i+1);
        if ( dist[i] == INT_MAX) {
            printf("?\t");
            printf("inf\t");
        } else {
            printf(" %-4d |", next_hop[i]+1);
            printf(" %-4d |", dist[i]);
        }
        printf("\n");
    }
    printf("+------+------+------+\n");
}

int main ()
{
    printf("\nDistance Vector Routing (Bellman Ford)\n");
    Graph* g= input_graph();

    // do bellman ford here
    for ( int i = 0; i < g-> num_nodes; i++) {
        bellman_ford(g, i);
    }

    /* print_graph(g); */
    return 0;
}
