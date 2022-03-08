#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 4305
#define BUF_SIZE 1024

int sock;
char *buffer;

int main() {


    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int r = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if ( r == -1) {
        printf("Error: Failed to connect()" );
        exit(1);
    }

    printf("Connected to server\n");
    buffer = malloc(sizeof(char) * BUF_SIZE);

    while(1) {
/*         printf("\nEnter the query: "); */
/*         fflush(stdout); */
/*         memset(buffer, 0, BUF_SIZE); */
/*         scanf("%[^\n]", buffer); */
/*         char t; scanf("%c", &t); */
/*         if ( strlen(buffer) == 0){ */
/*             continue; */
/*         } */

/*         send(sock, buffer, strlen(buffer), 0); */
/*         memset(buffer, 0, BUF_SIZE); */
/*         int r = recv(sock, buffer,BUF_SIZE , 0); */
/*         printf("[Server] %s", buffer); */
    }

    return 0;
}
