#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct _edge {
    int from;
    int to;
    int cost;
    struct _edge* next;
} Edge;

typedef struct _graph {
    Edge **adj_list;
    int num_nodes;
    int num_edges;
} Graph;

Graph* make_graph(int node_count) {
    Graph *g = malloc(sizeof(Graph));
    g->num_nodes = node_count;
    g->adj_list = malloc(sizeof(Edge*) * node_count);
    return g;
}

void add_edge(Graph *g, int from, int to, int cost) {

    assert(from >= 0);
    assert(to >= 0);

    Edge* t = g->adj_list[from];
    Edge* new_edge = malloc(sizeof(Edge));
    new_edge->from = from;
    new_edge->to = to;
    new_edge->cost = cost;

    g->num_edges++;

    if ( t == NULL) {
        g->adj_list[from] = new_edge;
        return;
    }

    while ( t->next != NULL) {
        t = t->next;
    }
    t->next = new_edge;

}

void print_graph(Graph *g) {
    printf("num_nodes = %d, num_edges = %d\n", g->num_nodes, g->num_edges);

    for ( int i = 0; i < g->num_nodes; i++) {
        printf("Node %d\n", i+1);
        Edge *t = g->adj_list[i];
        while (t != NULL ) {
            printf("\t from:%d to:%d cost:%d\n", t->from+1, t->to+1, t->cost);
            t = t->next;
        }
        printf("\n");
    }


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

    Graph *g = make_graph(num_nodes);

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

    /* print_graph(g); */
    return 0;
}
