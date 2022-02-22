#include<stdio.h>
#include<stdlib.h>

#define MAX_ITEMS 100

typedef struct _heap_item {
    int key;
    int value;
} HeapItem;

typedef struct _heap {
    int size;
    HeapItem* items;
} MinHeap;

MinHeap* make_heap() {
    MinHeap* h = malloc(sizeof(MinHeap));
    h->size = 0;
    h->items = malloc(sizeof(HeapItem) * MAX_ITEMS);
    return h;
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
    while ( i != 0 && h->items[(i-1)/2].key > h->items[i].key ) {
        heap_swap_items(&(h->items[(i-1)/2]), &(h->items[i]));
        i = (i-1)/2;
    }
}

void heap_heapify(MinHeap *h, int pos) {
    int left = 2*pos+1;
    int right = 2*pos +2;

    int smallest = pos;

    if ( left < h->size && h->items[left].key < h->items[smallest].key ) {
        smallest = left;
    }
    if ( right < h->size && h->items[right].key < h->items[smallest].key ) {
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
    if ( h->size == 1)
        return &(h->items[--h->size]);

    HeapItem* root = malloc(sizeof(HeapItem));

    heap_swap_items(root,&(h->items[0]));
    heap_swap_items(&(h->items[0]), &(h->items[--h->size]));
    heap_heapify(h, 0);

    return root;
}


int main(){
    MinHeap *h = make_heap();

    heap_insert(h,2,5);
    heap_insert(h,3,7);
    heap_insert(h,4,6);
    heap_insert(h,1,2);


    HeapItem *min = heap_extract_min(h);
    printf("Extract Min %d %d\n", min->key, min->value);
    free(min);

    min = heap_extract_min(h);
    printf("Extract Min %d %d\n", min->key, min->value);
    free(min);

    min = heap_extract_min(h);
    printf("Extract Min %d %d\n", min->key, min->value);
    free(min);

    min = heap_extract_min(h);
    printf("Extract Min %d %d\n", min->key, min->value);
    free(min);

}
