#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 8080
#define BUF_SIZE 1024
#define BACKLOG 5


char* get_file_contents(char *filename) {
    // it is given that file exists
    FILE *fptr = fopen(filename, "r");
    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char *file_contents = malloc(sizeof(char) * file_size + 1);
    fread(file_contents, file_size, 1, fptr);
    fclose(fptr);
    // add null
    file_contents[file_size] = 0;
    return file_contents;
}

void build_404_response(char *buffer) {
    char* file = get_file_contents("404.html");
    strcat(buffer, "HTTP/1.1 404 Not Found\r\n");
    strcat(buffer, "Server: anandu\r\n");
    strcat(buffer, "Content-Type: text/html\r\n");
    strcat(buffer, "\r\n");
    strcat(buffer, file);
    strcat(buffer, "\r\n");
    free(file);
}

void build_file_response(char *buffer, char* filename) {
    char* file = get_file_contents(filename);
    strcat(buffer, "HTTP/1.1 200 OK\r\n");
    strcat(buffer, "Server: anandu\r\n");
    strcat(buffer, "Content-Type: text/html\r\n");
    strcat(buffer, "\r\n");
    strcat(buffer, file);
    strcat(buffer, "\r\n");
    free(file);
}


void handle_client(int client_socket) {
    printf("Connected to Client\n");
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    int r = recv(client_socket, buffer, BUF_SIZE, 0);
    printf("Received \n");
    printf("%s", buffer);
    printf("\n");
    // TODO parse this buffer & extract filename

    memset(buffer, 0, BUF_SIZE);
    char filename[] = "index.html";
    if( access(filename, F_OK ) == 0 ) {
        build_file_response(buffer, filename);
    } else {
        build_404_response(buffer);
    }

    send(client_socket, buffer, strlen(buffer), 0);
    free(buffer);
    close(client_socket);
}

int main ()
{

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address));
    if ( success != 0 ) {
        printf("Bind failed");
        exit(-1);
    }

    listen(server_sock, BACKLOG);

    printf("Waiting for connections on %d\n", PORT);
    while (1) {
        int client_socket = accept(server_sock, NULL, NULL);
        if ( client_socket == -1 ) {
            continue;
        }
        if (!fork()) {
            close(server_sock);
            handle_client(client_socket);
        }
        close(client_socket);
    }

    close(server_sock);
    return 0;
}
