#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#define MAX_FRUITS 10

typedef struct _fruit {
    char* name;
    unsigned int count;
} Fruit;

void add_fruit(char *, int);
void print_inventory();
void purchase_fruit(char*, int);
void setup_fruits();
