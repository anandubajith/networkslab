#include "common.h"

extern Message* messageHead;
char command[BUF_SIZE];
int command_pos = 0;
int sock;
Packet p;
char *buffer;

void close_handler(int sig) {
    signal(sig, SIG_IGN);
    printf("\nClosing connection\n");
    if ( sock != -1) {
        strcpy(buffer, "Bye");
        send(sock, buffer, strlen(buffer), 0);
        shutdown(sock, 2);
        free(buffer);
    }
    exit(0);
}

void sync_data() {
    memset(buffer, 0, BUF_SIZE);
    int r = recv(sock, &p, sizeof(p), 0);
    if ( r == 0 )
        close_handler(0);
    if ( r == -1 )
        return;
    add_message(p.from, p.body);
}
void draw() {
    printf("\x1b[2J");
    printf("\x1b[?25l");
    printf("\x1b[H");
    Message *t = messageHead;
    while ( t != NULL ) {
        print_message(t);
        t = t->next;
    }
    printf("\x1b[7m");
    printf("\n#> :\x1b[m %s\n", command);
    fflush(stdout);
}

void process_keypress() {
    char c;
    if ( read(STDIN_FILENO, &c, 1) == 1) {
        if ( c == '\n' ) {
            if (strcmp("Bye", command) == 0 ) {
                add_message("??", "Bye");
                close_handler(0);
            }
            if ( strlen(command) != 0 ) {
                send(sock, command, strlen(command), 0);
            }
            bzero(command, sizeof(command));
            command_pos = 0;
        } else if ( c == 127) {
            if ( command > 0)
                command[--command_pos] = 0;
        } else {
            command[command_pos++] = c;
        }
    }

}

int main() {

    signal(SIGINT, close_handler);


    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;


    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    buffer = malloc(sizeof(char) * BUF_SIZE);

    printf("Enter NickName: ");
    scanf("%s", buffer);

    setup_terminal();

    int r = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if ( r == -1) {
        printf("Error: " );
        exit(1);
    }
    send(sock, buffer, strlen(buffer), 0);

    add_message("server", "Connected to server");

    while(1) {
        sync_data();
        draw();
        process_keypress();
    }

    return 0;
}
