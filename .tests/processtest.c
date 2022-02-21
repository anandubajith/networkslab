#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#define BUF_SIZE 1024

int create_file(char *filename) {
    char *cmd = malloc(sizeof(char) * BUF_SIZE);
    strcpy(cmd, "touch '");
    strcat(cmd, filename);
    strcat(cmd, "'");
    printf("cmd: %s\n", cmd);
    FILE *fp = popen(cmd, "r");
    free(cmd);
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return -1;
    }
    pclose(fp);
    return 0;
}

char* list_dir() {
    FILE* fp = popen("/bin/ls .", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return NULL;
    }
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0,BUF_SIZE);
    char tmp[BUF_SIZE];
    while (fgets(tmp,sizeof(tmp), fp) != NULL) {
        strcat(buffer,tmp);
    }

    pclose(fp);
    return buffer;
}


int main( int argc, char *argv[] )
{

  FILE *fp;
  char path[1035];

  create_file("Anandu");

  char* ls_out = list_dir();
  printf("%s", ls_out);


  return 0;
}
