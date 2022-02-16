#include "common.h"
/*
 * Client sends plaintext Message
 * First message from client = NICKNAME
 * Message starts with @ => send to username
 * Server sends message{from, to, time};
 */

typedef struct _user {
    char* name;
    int socket;
    struct _user *next;
} User;

User *usersHead = NULL;

int add_user(char *name, int socket) {
    User *u = malloc(sizeof(User));
    u->name = malloc(sizeof(char) * strlen(name)+ 1);
    strcpy(u->name, name);
    u->socket = socket;
    u->next = NULL;

    User *t = usersHead;
    if ( t == NULL ) {
        usersHead = u;
        return 0; // success
    }
    while ( t->next != NULL ) {
        if ( strcmp(t->name, name) == 0) {
            free(u->name);
            free(u);
            return -1; // NICK already exists
        }
        t = t->next;
    }
    t->next = u;
    return 0; // success
}

char* get_user(int socket) {
    User *t = usersHead;
    while ( t != NULL ) {
        if ( t->socket == socket )
            return t->name;
        t = t->next;
    }

    return NULL;
}

int remove_user(int socket) {
    User *t = usersHead;
    if ( t == NULL ) return 0;
    User *u;
    if ( t->socket == socket ) {
        u = t;
        usersHead = t->next;
    } else {
        while ( t->next != NULL && t->next->socket != socket) {
            t = t->next;
        }

        u = t->next;
        t->next =  u->next;
    }
    free(u->name);
    free(u);
    return 0;
}

#define MAX_CLIENTS 100


struct pollfd *poll_fds;
int fd_count;


void add_poll(char *name, int socket) {
    add_user("server", socket);

    poll_fds[fd_count].fd = socket;
    poll_fds[fd_count].events = POLLIN;

    fd_count++;
}

void remove_poll(int index) {
    // remove user based on socket
    remove_user(poll_fds[index].fd);
    poll_fds[index] = poll_fds[fd_count-1];
    fd_count--;
}



int main () {

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");


    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


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

    setup_terminal();

    poll_fds = malloc(sizeof(struct pollfd *) * MAX_CLIENTS);
    fd_count = 0;

    add_poll("server", server_sock);

    char *buffer = malloc(sizeof(char)*BUF_SIZE);

    while(1) {
        int r = poll(poll_fds, fd_count, -1);
        if ( r == -1){
            printf("Failed to poll\n");
            exit(-3);
        }
        for ( int i = 0; i < fd_count; i++) {
            if ( poll_fds[i].revents & POLLIN ) {
                memset(buffer, 0, BUF_SIZE);
                if ( poll_fds[i].fd == server_sock ) {
                    // handler server
                    // accept the connection
                    // recv username from client soccket
                    // add client socket to poll_fds
                } else {
                    int r = recv(poll_fds[i].fd, buffer, BUF_SIZE, 0);
                    if ( r > 0 ) {
                        // have data to read
                        // broadcast to all
                    } else if ( r == 0 ) {
                        // closed connection
                    } else {
                        // error from this => close
                    }
                }
            }
        }
    }

    return 0;
}
