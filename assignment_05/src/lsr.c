#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define MAX_ITEMS 100
#define BUF_SIZE 1000

typedef struct _heap_item {
    int key;
    int value;
} HeapItem;

typedef struct _heap {
    int size;
    HeapItem* items;
} MinHeap;

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


MinHeap* heap_create() {
    MinHeap* h = malloc(sizeof(MinHeap));
    h->size = 0;
    h->items = malloc(sizeof(HeapItem) * MAX_ITEMS);
    return h;
}

void heap_destroy(MinHeap *h) {
    free(h->items);
    free(h);
}

void heap_swap_items(HeapItem *x, HeapItem *y) {
    int t1 = x->key;
    int t2 = x->value;

    x->key = y->key;
    x->value = y->value;

    y->key = t1;
    y->value = t2;

}

void heap_insert(MinHeap *h, int key, int value) {
    if ( h->size == MAX_ITEMS ) {
        printf("Max items reached\n");
        exit(0);
    }


    h->items[h->size].key = key;
    h->items[h->size].value = value;
    h->size++;

    int i = h->size -1;
    while ( i != 0 && h->items[(i-1)/2].value > h->items[i].value) {
        heap_swap_items(&(h->items[(i-1)/2]), &(h->items[i]));
        i = (i-1)/2;
    }
}

void heap_heapify(MinHeap *h, int pos) {
    int left = 2*pos+1;
    int right = 2*pos +2;

    int smallest = pos;

    if ( left < h->size && h->items[left].value < h->items[smallest].value) {
        smallest = left;
    }
    if ( right < h->size && h->items[right].value < h->items[smallest].value ) {
        smallest = right;
    }
    if ( smallest != pos) {
        heap_swap_items(&(h->items[pos]), &(h->items[smallest]));
        heap_heapify(h, smallest);
    }
}


HeapItem* heap_extract_min(MinHeap *h) {
    if ( h->size <= 0)
        return NULL;


    HeapItem* root = malloc(sizeof(HeapItem));
    root->key = root->value = 0;

    if ( h->size == 1) {
        heap_swap_items(root,&(h->items[0]));
        h->size--;
        return root;
    }

    heap_swap_items(root,&(h->items[0]));
    heap_swap_items(&(h->items[0]), &(h->items[h->size-1]));
    h->size--;

    heap_heapify(h, 0);
    return root;
}

Graph* graph_create(int node_count) {
    Graph *g = malloc(sizeof(Graph));
    g->num_nodes = node_count;
    g->adj_list = malloc(sizeof(Edge*) * node_count);
    return g;
}

void graph_destroy(Graph* g) {
    free(g->adj_list);
    free(g);
}

void graph_add_edge(Graph *g, int from, int to, int cost) {

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

void graph_print(Graph *g) {
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



void build_path(char *s, int* prev, int* dist, int dest) {
    memset(s, 0, BUF_SIZE);

    // going to same node
    if ( dist[dest] == 0 ) {
        sprintf(s, "%d->%d", dest+1, dest+1);
        return;
    }

    if (dist[dest] == INT_MAX) {
        sprintf(s, "No path");
        return;
    }

    char tmp[BUF_SIZE];
    int start = dest;
    memset(tmp, 0, 100);
    sprintf(tmp, "%d>-", start+1);
    strcat(s, tmp);

    while ( prev[start] != INT_MAX) {
        memset(tmp, 0, 100);
        sprintf(tmp, "%d>-", prev[start]+1);
        strcat(s, tmp);
        start = prev[start];
    }
    int l = strlen(s);
    s[--l] = 0;
    s[--l] = 0;

    for ( int i = 0; i < l/2;i++) {
        char t= s[i];
        s[i] = s[l-i-1];
        s[l-i-1]= t;
    }
}

void dijkstra(Graph *g, int start) {

    int dist[g->num_nodes];
    int visited[g->num_nodes];
    int prev[g->num_nodes];

    for ( int i = 0; i < g->num_nodes; i++) {
        dist[i] = INT_MAX;
        prev[i] = INT_MAX;
        visited[i] = 0;
    }

    dist[start] = 0;

    MinHeap *h = heap_create();
    heap_insert(h, start, 0);

    while (h->size > 0 ) {
        HeapItem *min = heap_extract_min(h);
        visited[min->key] = 1;
        if ( dist[min->key] < min->value) continue;

        Edge* t = g->adj_list[min->key];
        while ( t != NULL ) {
            if ( visited[t->to] == 1 ) {
                t= t->next;
                continue;
            }
            int new_dist = dist[min->key] + t->cost;
            if ( new_dist < dist[t->to] ) {
                prev[t->to] = min->key;
                dist[t->to] = new_dist;
                heap_insert(h, t->to, new_dist);
            }
            t = t->next;
        }
        free(min);
    }

    /*
     * i can calculate max size of path box
     * 3*num_nodes -2
     */

    printf("\033[1m\033[37m");
    printf("\nLSR at node %d\n", start+1);
    printf("\033[0m");
    char *s = malloc(sizeof(char)* BUF_SIZE);

    printf("+---------------------+------+\n");
    printf("| path                | cost |\n");
    printf("+---------------------+------+\n");
    for ( int i = 0; i < g->num_nodes; i++) {
        /* printf("Destination: %d\n", i+1); */
        build_path(s, prev, dist, i);

        printf("| %-20s", s);
        if ( dist[i] == INT_MAX ) {
            printf("| inf  |\n");
        } else {
            printf("| %-4d |\n", dist[i]);
        }
    }
    printf("+---------------------+------+\n");

    free(s);
    heap_destroy(h);
}

int main () {
    printf("Link State Routing => Dijkstra\n");
    int num_nodes, num_edges;
    scanf("%d %d", &num_nodes, &num_edges);

    Graph *g = graph_create(num_nodes);

    int from, to, cost;
    for ( int i = 0; i < num_edges; i++) {
        scanf("%d %d %d", &from, &to, &cost);
        graph_add_edge(g, from-1, to-1, cost);
        // Graphs are undirected
        graph_add_edge(g, to-1, from-1, cost);
    }

    for ( int i = 0; i < g-> num_nodes; i++) {
        dijkstra(g, i);
    }
    graph_destroy(g);

    return 0;
}
