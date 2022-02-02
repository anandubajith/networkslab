#include "fruits.h"

// array of fruit pointers
Fruit** fruits;
int fruit_count;

Fruit* parse_fruit_string(char *s) {
    Fruit *f = malloc(sizeof(Fruit));

    int l = strlen(s);
    int point = l-1;
    while ( point >= 0 ) {
        if ( s[point] >= '0' && s[point] <= '9' ) {
            point--;
        } else {
            break;
        }
    }
    if ( point == l-1) {
        printf("Invalid message\n");
        return NULL;
    }
    /* printf("point: %d \n" , point); */
    /* printf("rest = %s\n", s + point +1); */

    f->name = malloc(sizeof(char) * point+1);
    strncpy(f->name,s, point+1);
    if ( f->name[point] == ' ' ) {
        f->name[point] = '\0';
    }
    f->count = atoi(s + point+1);

    /* printf("parsed fruitName: \"%s\"\n", f->name); */
    /* printf("parsed count: \"%d\"\n", f->count); */

    return f;
}

void add_fruit(char *name, int count) {
    Fruit *f = malloc(sizeof(Fruit));
    f->name = malloc((strlen(name)+1)*sizeof(char));
    strcpy(f->name, name);
    f->count = count;
    fruits[fruit_count++] = f;
}

void print_inventory(char *buf) {
    sprintf(buf, "\nInventory\n");
    for (int i =0; i < fruit_count; i++) {
        sprintf(buf + strlen(buf), "%s -> %d\n" , fruits[i]->name, fruits[i]->count);
    }
    sprintf(buf + strlen(buf), "Total %d items\n" , fruit_count);
}

int purchase_fruit(char *name, int count) {
    int f_index = -1;
    // todo: debug?
    for ( int i  = 0; i < fruit_count; i++) {
        if ( strcmp(fruits[i]->name, name) == 0) {
            f_index = i;
            break;
        }
    }
    if( f_index == -1 || fruits[f_index]->count < count) {
        printf("ERROR\n");
        return 0;
    }

    fruits[f_index]->count -= count;
    return 1; // success
}

void setup_fruits() {
    // sets up memory
    fruits = malloc(MAX_FRUITS * sizeof(Fruit*));
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


