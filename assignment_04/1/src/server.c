#include "common.h"

#define MAX_CLIENTS 100

typedef struct _user {
    char* name;
    int socket;
    struct _user *next;
} User;

struct pollfd *poll_fds;
int* active_clients;
int fd_count;
Packet p;
User *usersHead = NULL;

int add_user(char *name, int socket) {
    User *t = usersHead;

    User *prev = NULL;
    while ( t != NULL ) {
        if ( strcmp(t->name, name) == 0 ) {
            return -1;
        }
        prev = t;
        t = t->next;
    }

    User *u = malloc(sizeof(User));
    u->name = malloc(sizeof(char) * strlen(name)+ 1);
    strcpy(u->name, name);
    u->socket = socket;
    u->next = NULL;
    if ( prev == NULL ) {
        usersHead = u;
    } else {
        prev->next = u;
    }
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


void add_poll(int socket) {
    poll_fds[fd_count].fd = socket;
    poll_fds[fd_count].events = POLLIN;
    active_clients[fd_count] = 0;
    fd_count++;
}

void remove_poll(int index) {
    poll_fds[index] = poll_fds[fd_count-1];
    active_clients[index] = -1;
    fd_count--;
}


void broadcast(char *from, char *message){
    // starting from 1 to avoid server
    printf("Broadcast \"%s\" from \"%s\"\n", message, from);
    memset(&p, 0, sizeof(p));
    strcpy(p.from, from);
    strcpy(p.body, message);
    p.time = time(NULL);
    for ( int i = 1; i < fd_count; i++) {
        if ( active_clients[i] == 1 ) {
            send(poll_fds[i].fd, &p, sizeof(p), 0);
        }
    }
    /* printf("Broadcasting done\n"); */
}


int main () {

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");


    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
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
    active_clients = malloc(sizeof(int) * MAX_CLIENTS);
    memset(active_clients, 0, sizeof(int)*MAX_CLIENTS);
    fd_count = 0;

    add_poll(server_sock);
    add_user("server", server_sock);

    char buffer[BUF_SIZE];

    while(1) {
        int r = poll(poll_fds, fd_count, -1);
        if ( r == -1){
            printf("Failed to poll\n");
            exit(-3);
        }
        for ( int i = 0; i < fd_count; i++) {
            if ( poll_fds[i].revents & POLLIN ) {
                /* printf("Have data to read\n"); */
                memset(buffer, 0, BUF_SIZE);
                if ( poll_fds[i].fd == server_sock ) {
                    // handle 'accept' to server
                    int client_sock = accept(server_sock, NULL, NULL);
                    if ( client_sock == -1) {
                        printf("Failed to accept connection \n");
                    } else {
                        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
                        add_poll(client_sock);
                        printf("accepted connection\n");
                    }
                } else {
                    /* printf("Waiting for recv on socket %d %s\n", i, get_user(poll_fds[i].fd)); */
                    int r = recv(poll_fds[i].fd, buffer, BUF_SIZE, 0);
                    /* printf("Read completed\n"); */
                    if ( r > 0 ) {
                        if ( active_clients[i] == 0 ) {
                            // received nitckname
                            int rr = add_user(buffer, poll_fds[i].fd);
                            if ( rr == -1 ) {
                                // todo: tell that username already exists
                                printf("User already exists :(\n");
                                memset(&p, 0, sizeof(p));
                                strcpy(p.from, "server");
                                strcpy(p.body, "Username already in use");
                                p.time = time(NULL);
                                send(poll_fds[i].fd, &p, sizeof(p), 0);
                                shutdown(poll_fds[i].fd, 2);
                                remove_poll(i);
                            } else {
                                active_clients[i] = 1;
                                // broadcast [server] X has joined
                                memset(buffer,0, BUF_SIZE);
                                sprintf(buffer, "%s has joined the chat", get_user(poll_fds[i].fd));
                                broadcast("server", buffer);
                            }
                        } else {
                            // process the message
                            broadcast(get_user(poll_fds[i].fd), buffer);
                        }
                    } else {
                        // either connection closed || error
                        memset(buffer,0, BUF_SIZE);
                        sprintf(buffer, "%s has left the chat", get_user(poll_fds[i].fd));
                        close(poll_fds[i].fd);
                        remove_user(poll_fds[i].fd);
                        remove_poll(i);
                        broadcast("server", buffer);
                    }
                }
            }
        }
    }

    return 0;
}
