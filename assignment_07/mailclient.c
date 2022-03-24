#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_SIZE 100
#define BUF_SIZE 1024

int get_socket_connection(int port) {
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
    sscanf(buffer + 3, "%d", &total_bytes);
    printf("\nTotal bytes = %d\n\n", total_bytes);

    // receive the rest of message?
    memset(buffer, 0, BUF_SIZE);

    while (total_bytes > 0) {
        memset(buffer, 0, BUF_SIZE);
        int r = recv(socket, buffer, BUF_SIZE, 0);
        printf("%s", buffer);
        if (r <= 0)
            break;
        total_bytes -= r;
    }
    free(buffer);

    char input[100];
    scanf(" %s", input);
    if (input[0] == 'd') {
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
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0); // ready message

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "USER %s", username);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("received: '%s'\n", buffer);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "PASS %s", password);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("received: '%s'\n", buffer);

    char input[100];
    int message_index;
    while (1) {
        printf("\x1b[2J\x1b[H");
        printf("Handle manage email\n");
        printf("List messages\n");
        printf("Sl. No. <Sender's email id> <received time; date: hour: minute> "
                "<Subject> \n");
        scanf("%s", input);
        printf("Got input %s\n", input);
        if (input[0] == 'q') {
            break;
        } else if (sscanf(input, "%d", &message_index) == 1) {
            /* printf("received input %d \n", message_index); */
            handle_view_message(socket, message_index);
        } else {
            /* printf("something else? %s\n", input); */
        }
    }
    free(buffer);
}
void send_email(int socket, char *from, char*to, char*subject, char*body, char*buffer){

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "MAIL FROM:<%s>", from);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    if ( buffer[0] == '5' ) {
        printf("Incorrect format for from address\n");
        return;
    }

    printf("Received : '%s'\n", buffer);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "RCPT TO:<%s>", from);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    if ( buffer[0] == '5' ) {
        printf("Incorrect format for to address\n");
        return;
    }

    printf("Received : '%s'\n", buffer);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "DATA");
    send(socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "subject: %s\n", subject);
    send(socket, buffer, strlen(buffer), 0);
    send(socket, body, strlen(body), 0);
    send(socket, ".", 1, 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("Received : '%s'\n", buffer);
    if ( buffer[0] == '5' ) {
        printf("Error when trying to put in data mode");
        return;
    }
}

void handle_send_mail(int server_port, char *username, char *password) {
    printf("\x1b[2J\x1b[H");
    printf("Handle send email\n");

    char *from = malloc(sizeof(char) * BUF_SIZE);
    char *to = malloc(sizeof(char) * BUF_SIZE);
    char *subject = malloc(sizeof(char) * BUF_SIZE);
    char *body = malloc(sizeof(char) * BUF_SIZE * 1000);
    memset(body, 0, BUF_SIZE * 1000);

    char t;
    scanf("%c", &t);

    printf("from: ");
    fflush(stdout);
    scanf("%[^\n]", from);
    scanf("%c", &t);

    printf("to: ");
    fflush(stdout);
    scanf("%[^\n]", to);
    scanf("%c", &t);

    printf("subject: ");
    fflush(stdout);
    scanf("%[^\n]", subject);
    scanf("%c", &t);

    printf("body: \n");
    fflush(stdout);
    char *temp = malloc(sizeof(char) * BUF_SIZE);
    while (1) {
        memset(temp, 0, BUF_SIZE);
        scanf("%[^\n]%c", temp, &t);
        /* printf("read: '%s' %ld", temp, strlen(temp)); */
        if ( strlen(body) != 0 )
            strcat(body, "\n");
        strcat(body, temp);
        if (strlen(temp) == 1 && temp[0] == '.') {
            printf("Received end marker\n");
            break;
        }
    }
    free(temp);

    /* printf("\nReceived input:\n"); */
    /* printf("from: '%s'\n", from); */
    /* printf("to: '%s'\n", to); */
    /* printf("subject: '%s'\n", subject); */
    /* printf("body: '%s'\n", body); */

    int socket = get_socket_connection(server_port);
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    send_email(socket, from, to, subject, body, buffer);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "QUIT");
    send(socket, buffer, strlen(buffer),0);
    shutdown(socket, 2);
    free(buffer);
    scanf("%c", &t);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
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
    while (1) {
        printf("\x1b[2J\x1b[H"); // clear and position cursor at top
        printf("Welcome %s\n", "asdf");
        printf("1. Manage mail\n2. Send Mail\n3. Quit\n");
        scanf("%c", &input);
        /* printf("got input %d", input); */
        if (input == '1') {
            handle_manage_mail(atoi(argv[2]), username, password);
        } else if (input == '2') {
            handle_send_mail(atoi(argv[1]), username, password);
        } else if (input == '3') {
            return 0;
        }
        input = 0;
    }

    return 0;
}
