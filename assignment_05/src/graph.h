#include <stdlib.h>
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


Graph* make_graph(int,int);
Graph* input_graph();
void add_edge(Graph* g, int, int, int);
void print_graph(Graph*);

// I'm holding graph in Nodes[N] list
// each node will have

