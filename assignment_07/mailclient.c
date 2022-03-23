#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 100
#define BUF_SIZE 1024

int get_socket_connection(int port ){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int r = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (r == -1) {
        printf("Error: Failed to connect()");
        exit(1);
    }

    return sock;
}

void handle_view_message(int socket, int message_index) {

    printf("\x1b[2J\x1b[H");
    printf("\nMessage: %d\n", message_index);
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "RETR %d", message_index);
    send(socket, buffer, strlen(buffer), 0);
    // receive the message?
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("Received '%s' \n", buffer);

    int total_bytes = 0;
    sscanf(buffer+3, "%d", &total_bytes);
    printf("\nTotal bytes = %d\n\n", total_bytes);

    // receive the rest of message?
    memset(buffer, 0, BUF_SIZE);

    while ( total_bytes > 0 ) {
        memset(buffer, 0, BUF_SIZE);
        int r = recv(socket, buffer, BUF_SIZE, 0);
        printf("%s", buffer);
        if ( r <= 0)
            break;
        total_bytes -= r;
    }
    free(buffer);

    char input[100];
    scanf(" %s", input);
    if ( input[0] == 'd' ) {
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "DELE %d", message_index);
        send(socket, buffer, strlen(buffer), 0);
    }
}

void handle_manage_mail(int server_port, char *username, char *password) {
    // login to pop server
    //
    int socket = get_socket_connection(server_port);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE,0); // ready message

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "USER %s", username);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("received: '%s'\n",buffer);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "PASS %s", password);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("received: '%s'\n",buffer);




    char input[100];
    int message_index;
    while(1) {
        printf("\x1b[2J\x1b[H");
        printf("Handle manage email\n");
        printf("List messages\n");
        printf("Sl. No. <Sender's email id> <received time; date: hour: minute> <Subject> \n");
        scanf("%s", input);
        printf("Got input %s\n", input);
        if ( input[0] == 'q') {
            break;
        } else if( sscanf(input, "%d", &message_index ) == 1 ) {
            /* printf("received input %d \n", message_index); */
            handle_view_message(socket, message_index);
        } else {
            /* printf("something else? %s\n", input); */
        }

    }
    free(buffer);
}

void handle_send_mail(int server_port, char *username, char *password) {
    // create socket connection to the SMTP server
    printf("\x1b[2J\x1b[H");
    printf("Handle send email\n");


    char *from = malloc(sizeof(char) * BUF_SIZE);
    char *to = malloc(sizeof(char) * BUF_SIZE);
    char *subject = malloc(sizeof(char) * BUF_SIZE);
    char *body = malloc(sizeof(char) * BUF_SIZE);

    printf("from: ");
    fflush(stdout);
    scanf("%s", from);

    printf("to: ");
    fflush(stdout);
    scanf("%s", to);

    printf("subject: ");
    fflush(stdout);
    scanf("%s", subject);


    // input from, to, subject
    // take input till receiving . for body
    //
    // try to init socket and send email
    // if invalid => incorrect format message
    //
    // else fail on erro
    // in the end the "Mail send successfully"
    // show <ENTER>


}

int main (int argc, char *argv[]) {
    if ( argc < 3 ) {
        printf("./mailclient SMTP_PORT POP_PORT\n");
        return 1;
    }

    char *username = malloc(sizeof(char) * MAX_SIZE);
    char *password = malloc(sizeof(char) * MAX_SIZE);

    // ask for username
    printf("username: ");
    fflush(stdout);
    scanf("%s", username);
    // ask for password
    printf("password: ");
    fflush(stdout);
    scanf("%s", password);


    char input = 0;
    while(1){
        printf("\x1b[2J\x1b[H"); // clear and position cursor at top
        printf("Welcome %s\n", "asdf");
        printf("1. Manage mail\n2. Send Mail\n3. Quit\n");
        scanf("%c", &input);
        /* printf("got input %d", input); */
        if ( input == '1') {
            handle_manage_mail(atoi(argv[2]), username, password);
        } else if ( input == '2') {
            handle_send_mail(atoi(argv[1]), username, password);
        } else if ( input == '3') {
            return 0;
        }
        input = 0;
    }


    return 0;
}
