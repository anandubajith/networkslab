#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 100
#define BUF_SIZE 512

typedef struct _mail {
    char *from;
    char *to;
    char *subject;
    char *received_time;
    char *body;
} Mail;


void list_messages(char *username) {
    char *path = malloc(sizeof(char) * MAX_SIZE * 2);
    sprintf(path, "./%s/mymailbox", username);
    FILE *fp = fopen(path, "r");

    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    // three states
    // - mail start
    // - headers
    // - body

    while ((read = getline(&line, &len, fp)) != -1) {
        /* printf(line); */
    }

    if (line) free(line);

}

int main (int argc, char *argv[])
{
    if ( argc < 2 ) {
        printf("./popserver PORT\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    printf("Pop server\n");

    list_messages("anandu");


    return 0;
}
