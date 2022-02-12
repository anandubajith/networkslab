#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 500

int main()
{
    int num;
    FILE *fptr;

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("../file.bin","r");

    if(fptr == NULL) {
        printf("Error!");
        exit(1);
    }



    char* buffer = malloc(sizeof(char) * BUF_SIZE);



    /* fseek(fptr, 0, SEEK_SET); */
    /* fread(buffer, BUF_SIZE, 1, fptr); */
    int d = 0;
    while ( fread(buffer, BUF_SIZE, 1, fptr) == 1) {
        printf("BLOCK %d\n", d);
        /* printf("%s\n", buffer); */
        d++;
    }
    printf("Blocks read\n");
    int r = fread(buffer, BUF_SIZE, 1, fptr);
    printf("%s\n", buffer);
    printf("Last block had size = %d", d);



    fclose(fptr);
    return 0;
}
