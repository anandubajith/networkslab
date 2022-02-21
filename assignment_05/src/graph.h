#include <stdlib.h>
#include <stdio.h>

typedef struct _edge {
    int from;
    int to;
    int cost;
    struct _edge *next;
} Edge;

typedef struct _graph {
    Edge **adj_list;
    int num_nodes;
} Graph;


Graph* make_graph(int);
Graph* input_graph();
void add_edge(Graph* g, int, int, int);
void print_graph(Graph*);

// I'm holding graph in Nodes[N] list
// each node will have

