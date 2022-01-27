#include "fruits.h"

Fruit* fruits;
int fruit_count;

void add_fruit(char *name, int count) {
    Fruit *f = malloc(sizeof(Fruit));
    f->name = malloc((strlen(name)+1)*sizeof(char));
    strcpy(f->name, name);
    f->count = count;
    fruits[fruit_count++] = *f;
}

void print_inventory() {
    for (int i =0; i < fruit_count; i++) {
        printf("%s -> %d\n" , fruits[i].name, fruits[i].count);
    }
}


void purchase_fruit(char *name, int count) {
    // let this return message string -> SUCCESS | NOT AVAILABLE
    //
    int f_index = -1;
    // todo: debug?
    for ( int i  = 0; i < fruit_count; i++) {
        if ( strcmp(fruits[i].name, name) == 0) {
            f_index = i;
            break;
        }
    }

    if ( fruits[f_index].count < count ) {
        printf("ERROR");
        return;
    }

    fruits[f_index].count -= count;
    return;
}

void setup_fruits() {
    // sets up memory
    fruits = malloc(MAX_FRUITS * sizeof(Fruit));
    fruit_count = 0;

    // loads the default fruit inventory
    add_fruit("apple", 42);
    add_fruit("mango", 7);
    add_fruit("banana", 5);
    add_fruit("chikoo", 9);
    add_fruit("papaya", 2);

}

void cleanup_inventory() {
    // clean up each fruit
    // clean up fruits array
}


