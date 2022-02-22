#include "graph.h"

Graph* make_graph(int node_count, int edge_count) {
    Graph* g = malloc(sizeof(Graph));
    g->num_nodes = node_count;
    g->edges = malloc(sizeof(Edge) * edge_count*2);
    g->num_edges = 0;
    return g;
}


void add_edge(Graph* g, int from, int to, int cost) {
    g->edges[g->num_edges].cost = cost;
    g->edges[g->num_edges].from = from;
    g->edges[g->num_edges].to = to;
    g->num_edges++;
}


Graph* input_graph(){
    int num_nodes, num_edges;
    scanf("%d %d", &num_nodes, &num_edges);

    Graph *g = make_graph(num_nodes, num_edges);

    int from, to, cost;
    for ( int i = 0; i < num_edges; i++) {
        scanf("%d %d %d", &from, &to, &cost);
        add_edge(g, from-1, to-1, cost);
        // todo: For undirected
        add_edge(g, to-1, from-1, cost);
    }

    return g;
}

void print_graph(Graph *g) {
    printf("num_nodes = %d, num_edges = %d \n", g->num_nodes, g->num_edges);
    for (int i = 0; i < g->num_edges; i++) {
        printf("Edge: %d %d %d\n", g->edges[i].from, g->edges[i].to, g->edges[i].cost);
    }
}


