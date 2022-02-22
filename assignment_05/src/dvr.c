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
    next_hop[start] = start; //todo:check

    for ( int i = 0; i < g->num_nodes; i++) {
        Edge *t = g->adj_list[i];
        while ( t != NULL) {
            /* printf("Considering edge from %d to %d with cost %d\n", t->from, t->to, t->cost); */
            if ( dist[t->from] != INT_MAX && dist[t->from] + t->cost < dist[t->to]) {
                dist[t->to] = dist[t->from] + t->cost;
                next_hop[t->to] = t->from;
            }
            t = t->next;
        }
    }
    // todo: rerunning the above loop fixes it, so check

    // print the routing tables for each router

    printf("\n\nRouting Table at node %d\n", start+1);
    printf("dest\tnext\tcost\n");
    for ( int i = 0; i < g->num_nodes; i++) {
        printf("%d\t", i+1);
        if ( dist[i] == INT_MAX) {
            printf("?\t");
            printf("inf\t");
        } else {
            printf("%d\t", next_hop[i]+1);
            printf("%d\t", dist[i]);
        }
        printf("\n");
    }
    printf("\n---\n");
}

int main ()
{
    printf("Distance Vecctor Routing => Bellman Ford\n");
    Graph* g= input_graph();

    // do bellman ford here

    for ( int i = 0; i < g-> num_nodes; i++) {
        bellman_ford(g, i);
    }

    /* print_graph(g); */
    return 0;
}
