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


Graph* graph_create(int node_count, int edge_count) {
    Graph* g = malloc(sizeof(Graph));
    g->num_nodes = node_count;
    g->edges = malloc(sizeof(Edge) * edge_count*2);
    g->num_edges = 0;
    return g;
}

void graph_destroy(Graph*g) {
    free(g->edges);
    free(g);
}

void graph_add_edge(Graph* g, int from, int to, int cost) {

    assert(from >= 0);
    assert(to >= 0);

    g->edges[g->num_edges].cost = cost;
    g->edges[g->num_edges].from = from;
    g->edges[g->num_edges].to = to;
    g->num_edges++;
}

void graph_print(Graph *g) {
    printf("num_nodes = %d, num_edges = %d \n", g->num_nodes, g->num_edges);
    for (int i = 0; i < g->num_edges; i++) {
        printf("Edge: %d %d %d\n", g->edges[i].from, g->edges[i].to, g->edges[i].cost);
    }
}

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
                if ( e.from == start) {
                    // this means it's a direct link
                    next_hop[e.to] = e.to;
                } else {
                    // to store the path where we came from
                    next_hop[e.to] = next_hop[e.from];
                }
            }
        }
    }

    for ( int i =0; i < g->num_edges; i++) {
        Edge e = g->edges[i];
        if ( dist[e.from] != INT_MAX && dist[e.from] + e.cost < dist[e.to]) {
            // this is impossible in a physical network
            printf("Negative weight cycle exists in graph\n");
            return;
        }
    }


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
            printf(" ?    |");
            printf(" inf  |");
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
    int num_nodes, num_edges;
    scanf("%d %d", &num_nodes, &num_edges);

    Graph *g = graph_create(num_nodes, num_edges);

    int from, to, cost;
    for ( int i = 0; i < num_edges; i++) {
        scanf("%d %d %d", &from, &to, &cost);
        graph_add_edge(g, from-1, to-1, cost);
        // Graphs are undirected
        graph_add_edge(g, to-1, from-1, cost);
    }

    for ( int i = 0; i < g-> num_nodes; i++) {
        bellman_ford(g, i);
    }

    graph_destroy(g);

    return 0;
}
