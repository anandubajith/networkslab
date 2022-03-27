#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/tcp.h>
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

void handle_view_message(int socket, int message_index, int*delete_index) {

    printf("\x1b[2J\x1b[H");
    printf("\n\033[1m\033[37m\033[4mMessage: %d\n\033[0m", message_index);
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    int errored = 0;

    if ( delete_index[message_index] == 1) {
        printf("Message has been marked deleted\n");
    } else {
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "RETR %d\r\n", message_index);
        send(socket, buffer, strlen(buffer), 0);
        // receive the message?
        memset(buffer, 0, BUF_SIZE);
        recv(socket, buffer, BUF_SIZE, 0);
        /* printf("Received '%s' \n", buffer); // todo: parse this and force this much bytes to be read? */

        if ( buffer[0] == '+' ) {
            int total_bytes = 0;
            sscanf(buffer + 3, "%d", &total_bytes);
            printf("Total octets = %d\n\n", total_bytes);

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
        } else {
            printf("Error occurred\n");
            printf("%s", buffer);
            printf("\n");
            errored = 1;
        }

    }

    printf("---\n");
    if ( delete_index[message_index] != 1 && errored != 1)
        printf("\x1b[3mPress 'd' to mark email as deleted\n\x1b[23m");
    printf("\x1b[3mPress any key to return to message list\x1b[23m\n");

    char input[100];
    scanf(" %s", input);
    if (input[0] == 'd' && errored != 1 && delete_index[message_index] != 1) {
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "DELE %d\r\n", message_index);
        send(socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUF_SIZE);
        usleep(10000);
        recv(socket, buffer, BUF_SIZE, 0);
        if ( buffer[0] == '+' )
            delete_index[message_index] = 1;
        usleep(10000);
    }
    free(buffer);
}

void handle_manage_mail(int server_port, char *username, char *password) {
    // login to pop server
    //
    int socket = get_socket_connection(server_port);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    if (setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
        printf("setsocketopt(SO_RCVTIMEO) failed");


    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0); // ready message

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "USER %s\r\n", username);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    if ( buffer[0] == '-' ) {
        printf("Error occurred\n");
        printf("%s\n", buffer);
        exit(-1);
    }

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "PASS %s\r\n", password);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    if ( buffer[0] == '-' ) {
        printf("Error occurred\n");
        printf("%s\n", buffer);
        exit(-1);
    }


    char input[100];
    int message_index;
    int total_message_count = 0;
    int total_size = 0;
    int delete_index[100];
    memset(delete_index, 0, 100 * sizeof(int));
    while (1) {
        printf("\x1b[2J\x1b[H");
        printf("\x1b[1;31mManage Mail\n\n\x1b[0m");
        // send stat and get email count
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "STAT\r\n");
        if ( send(socket, buffer, strlen(buffer), 0) <= 0 ) {
            printf("Error sending STAT");
            break;
        }
        memset(buffer, 0, BUF_SIZE);
        if ( recv(socket, buffer, BUF_SIZE, 0) <= 0 ) {
            printf("Error receiving STAT");
            break;

        }
        /* printf("Got responseSTAT= %s\n\n", buffer); */
        if ( buffer[0] == '-' ) {
            printf("Error occurred\n");
            printf("%s\n", buffer);
        } else {
            if ( sscanf(buffer+3, "%d %d", &total_message_count, &total_size) != 2 ) {
                printf("Error occurred: Invalid response from server\n");
                break;
            }
            int printed_messages = 0;
            for ( int i = 1; i <= total_message_count; i++) {
                if ( delete_index[i] == 1) {
                    continue;
                }
                printf("\033[1m\033[37m\033[4mMessage: %d\n\033[0m", i);
                memset(buffer, 0, BUF_SIZE);
                sprintf(buffer, "TOP %d 4\r\n", i);
                send(socket, buffer, strlen(buffer), 0);

                char *temp = malloc(sizeof(char) * BUF_SIZE * 5);
                memset(temp, 0, BUF_SIZE *5);
                int line_count = 5;
                while ( line_count > 0 ) {
                    memset(buffer, 0, BUF_SIZE);
                    recv(socket, buffer, BUF_SIZE, 0);
                    /* printf("received after top: '%s'\n", buffer); */
                    for ( int i =0; i< BUF_SIZE; i++) {
                        if ( buffer[i] == '\n')
                            line_count--;
                    }
                    strcat(temp, buffer);
                }
                printf("%s", temp + 26);
                printf("\n");
                free(temp);
                printed_messages++;
            }

            if ( printed_messages  == 0 ) {
                printf("\nNo messages in mailbox\n");
                printf("\n");
            }
        }
        scanf("%s", input);
        printf("Got input %s\n", input);

        if (input[0] == 'q') {
            memset(input, 0, 100);
            sprintf(input, "QUIT\r\n");
            send(socket, input, strlen(input), 0);
            shutdown(socket, 2);
            break;
        } else if (sscanf(input, "%d", &message_index) == 1
                        &&  message_index > 0
                        && message_index <= total_message_count) {
            /* printf("received input %d \n", message_index); */
            handle_view_message(socket, message_index, delete_index);
        } else {
            /* printf("something else? %s\n", input); */
        }
    }
    free(buffer);
}
void send_email(int socket, char *from, char*to, char*subject, char*body, char*buffer){

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "MAIL FROM:<%s>\r\n", from);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    if ( buffer[0] == '5' ) {
        printf("Incorrect format for from address\n");
        printf("ERROR: %s\n",buffer);
        return;
    }

    printf("Received : '%s'\n", buffer);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "RCPT TO:<%s>\r\n", to);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    if ( buffer[0] == '5' ) {
        printf("Incorrect format for to address\n");
        printf("ERROR: %s\n",buffer);
        return;
    }

    printf("Received : '%s'\n", buffer);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "DATA\r\n");
    send(socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "subject: %s\r\n", subject);
    send(socket, buffer, strlen(buffer), 0);

    char *token = strtok(body, "\n");
    while ( token != NULL  ) {
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "%s\r\n", token);
        send(socket, buffer, strlen(buffer), 0);
        token = strtok(NULL, "\n");
    }
    send(socket, ".\r\n", 3, 0);

    memset(buffer, 0, BUF_SIZE);
    recv(socket, buffer, BUF_SIZE, 0);
    printf("Received : '%s'\n", buffer);
    if ( buffer[0] == '5' ) {
        printf("Error when trying to put in data mode");
        printf("ERROR: %s\n",buffer);
        return;
    }

    printf("Mail sent successfully");

}

void handle_send_mail(int server_port, char *username, char *password) {
    printf("\x1b[2J\x1b[H");
    printf("\x1b[1;32mSend Email\n\n\x1b[0m");

    char *from = malloc(sizeof(char) * BUF_SIZE);
    memset(from, 0, BUF_SIZE);
    char *to = malloc(sizeof(char) * BUF_SIZE);
    memset(to, 0, BUF_SIZE);
    char *subject = malloc(sizeof(char) * BUF_SIZE);
    memset(subject, 0, BUF_SIZE);
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
        if (strlen(temp) == 1 && temp[0] == '.') {
            printf("\nSendign email\n");
            break;
        }
        if ( strlen(body) != 0 )
            strcat(body, "\n");
        strcat(body, temp);
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
    sprintf(buffer, "QUIT\r\n");
    send(socket, buffer, strlen(buffer),0);
    shutdown(socket, 2);
    free(buffer);
    free(from);
    free(to);
    free(subject);
    free(body);

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
        printf("Welcome %s\n\n", username);
        printf("\x1b[1;31m1. Manage mail\n\x1b[1;32m2. Send Mail\n\x1b[1;33m3. Quit\n\x1b[1;0m# ");
        scanf("%c", &input);
        /* printf("got input %d", input); */
        if (input == '1') {
            handle_manage_mail(atoi(argv[2]), username, password);
        } else if (input == '2') {
            handle_send_mail(atoi(argv[1]), username, password);
        } else if (input == '3') {
            free(username);
            free(password);
            return 0;
        }
        input = 0;
    }

    return 0;
}
