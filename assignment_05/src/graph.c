#include "graph.h"

Graph* make_graph(int node_count) {
    Graph* g = malloc(sizeof(Graph));
    g->num_nodes = node_count;
    g->adj_list = malloc(sizeof(Edge*)* node_count);
    return g;
}


void add_edge(Graph* g, int from, int to, int cost) {
    Edge* e = malloc(sizeof(Edge));
    e->cost = cost;
    e->from = from;
    e->to = to;
    e->next = NULL;

    Edge *t = g->adj_list[from];
    if ( t == NULL) {
        g->adj_list[from] = e;
        return;
    }

    while ( t->next != NULL) {
        t = t->next;
    }
    t->next = e;

}


Graph* input_graph(){
    int num_nodes, num_edges;
    scanf("%d %d", &num_nodes, &num_edges);

    Graph *g = make_graph(num_nodes);

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
    for ( int i =0; i < g->num_nodes; i++) {
        printf("---\nNode %d\n", i+1);
        Edge *t = g->adj_list[i];
        while ( t != NULL ) {
            printf("\t%d (%d) \n",t->to+1, t->cost);
            t= t->next;
        }

        printf("---\n\n");
    }
}


