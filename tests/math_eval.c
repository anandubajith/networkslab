#include <stdio.h>
#include <ctype.h>

int is_operator(char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
            return 1;
        default:
            return 0;
    }
}
int parse_int(char*s, int i, int j) {
    return  0;
}

int evaluate(char* s ) {
    // parse integer operator integer
    int index = 0;
    while ( s[index] != '\0' ) {
        if ( isdigit(s[index]) || s[index] == ' ' ) {
            index++;
        } else {
            return -1;
        }
    }
    //
    int end = 0;
    int integer1 = parse_int(s, 0, index);
    int integer2 = parse_int(s, index+2,end );

    char operator = '+';

    switch(operator) {
        case '+':
            return integer1 + integer2;
        case '-':
            return integer1 + integer2;
        case '/':
            return integer1 / integer2; // TODO Handle integer2 == 0 case
        case '*':
            return integer1 * integer2;

    }

    return 0;
}
int main (int argc, char *argv[])
{
    char*s = "2 + 3";

    printf("%s evalutes to %d\n", s, evaluate(s));
    return 0;
}
