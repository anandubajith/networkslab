#include <stdio.h>

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

int evaluate(char* s ) {
    int integer1 = 0;
    int integer2 = 0;
    // parse integer operator integer
    //
    int index = 0;
    while ( s[index] != '\0' ) {
        if ( is_digit(s[index] ) ) {
        }
    }


}
int main (int argc, char *argv[])
{
    char*s = "2 + 3";

    printf("%s evalutes to %d\n", s, evaluate(s));
    return 0;
}
