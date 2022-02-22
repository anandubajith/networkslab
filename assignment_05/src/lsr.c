#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct _edge {
    int from;
    int to;
    int cost;
} Edge;

typedef struct _graph {
    Edge *edges;
    int num_nodes;
    int num_edges;
} Graph;

Graph* make_graph(int node_count, int edge_count) {
    return NULL;
}

void add_edge(Graph *g, int from, int to, int cost) {

}

void print_graph(Graph *g) {

}

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
    int num_nodes, num_edges;
    scanf("%d %d", &num_nodes, &num_edges);

    Graph *g = make_graph(num_nodes, num_edges);

    int from, to, cost;
    for ( int i = 0; i < num_edges; i++) {
        scanf("%d %d %d", &from, &to, &cost);
        add_edge(g, from-1, to-1, cost);
        // Graphs are undirected
        add_edge(g, to-1, from-1, cost);
    }

    for ( int i = 0; i < g-> num_nodes; i++) {
        dijkstra(g, i);
    }

    print_graph(g);
    return 0;
}
