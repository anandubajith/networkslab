#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <assert.h>

#define BACKLOG 5
#define MAX_SIZE 100
#define BUF_SIZE 512
#define HOSTNAME "anandu.local"

typedef struct _user {
    char*name;
    char*password;
    struct _user* next;
} User;

User* usersHead = NULL;

int add_user(char *username, char *password){
    User* u = malloc(sizeof(User));
    u->name = malloc(sizeof(char) * MAX_SIZE);
    strcpy(u->name, username);
    u->password = malloc(sizeof(char) * MAX_SIZE);
    strcpy(u->password , password);

    if ( usersHead == NULL ) {
        usersHead = u;
        return 0;
    }

    User *t = usersHead;
    while ( t->next != NULL) {
        if ( strcmp(t->name, username) == 0) {
            // username already exists
            free(u->name);
            free(u->password);
            free(u);
            return 1;
        }
        t = t->next;
    }

    t->next = u;
    return 0;
}

int check_username(char *username) {
    User *t = usersHead;
    while ( t != NULL) {
        if ( strcmp(t->name, username) == 0) {
            // username already exists
            return 0;
        }
        t = t->next;
    }
    return 1;
}

int check_password(char* username, char* password) {
    User *t = usersHead;
    while ( t != NULL) {
        if ( strcmp(t->name, username) == 0) {
            if ( strcmp(t->password, password) == 0 ) {
                // successful authentication
                return 0;
            }
            // incorrect username
            return 1;
        }
        t = t->next;
    }
    // invalid username
    return 2;
}

void print_users() {
    User *t = usersHead;
    while ( t != NULL ) {
        printf("username:%s password:%s\n", t->name, t->password);
        t = t->next;
    }
}

void create_folders() {
    struct stat st = {0};
    User *t = usersHead;
    char *buffer = malloc(sizeof(char) * MAX_SIZE);
    while ( t != NULL) {
        memset(buffer, 0, MAX_SIZE);
        sprintf(buffer, "./%s", t->name);
        if (stat(buffer, &st) == -1) {
            mkdir(buffer, 0700);
        }
        t = t->next;
    }

}
int load_usersfile() {
    FILE *fp = fopen("./logincred.txt", "r");
    if ( fp == NULL)
        return -1;

    char line[2 * MAX_SIZE + 1]; // username + , + password
    char username[MAX_SIZE];
    char password[MAX_SIZE];

    while (fgets(line, sizeof(line), fp)) {
        // split line by ,
        memset(username, 0, MAX_SIZE);
        memset(password, 0, MAX_SIZE);
        int i = 0;
        while ( line[i] != EOF && line[i] != ',' ) {
            i++;
        }
        if (line[i] == EOF) {
            fprintf(stderr, "Malformed line: %s\n", line);
            continue;
        }
        strncpy(username, line, i);
        strcpy(password, line+i+1);
        password[strlen(password)-1] = '\0';
        add_user(username, password);
        /* printf("'%s':'%s'\n", username, password); */
    }
    return 0;
}

int starts_with(char *string, char *marker) {
    assert( string != NULL );
    assert( marker != NULL );
    return strncmp(string, marker, strlen(marker)) == 0;
}

void send_reply(int socket, int code, char*message) {
    char *temp = malloc(sizeof(char) * BUF_SIZE);
    memset(temp, 0, BUF_SIZE);
    sprintf(temp, "%d %s\n", code, message);
    printf("sending reply '%s'\n", temp);
    send(socket, temp, strlen(temp), 0);
    free(temp);
}

typedef struct _state{
    char *from;
    char *to;
    char *body;
    int current;
} State;

void handle_cmd_helo(int socket, char *command, State *state) {
    // clear state
    memset(state, 0 , sizeof(State) );
    send_reply(socket, 250, HOSTNAME);
}
void handle_cmd_mail(int socket, char *command , State *state) {
    assert(state != NULL);
    if ( !starts_with("MAIL FROM:", command) ) {
        // invlaid command reply
        /* send_reply(socket) */
    }

    state->from = malloc(sizeof(char) * MAX_SIZE);

    strncpy(state->from, command+10, strlen(command)-10-1);
    printf(">> Received from '%s'\n", state->from);
    assert(strlen(state->from) != 0);

    send_reply(socket, 250, "OK");
}
void handle_cmd_rcpt(int socket, char *command, State *state) {
    assert(state != NULL);
    if ( !starts_with("RCPT TO:", command) ) {
        // invlaid command reply
        /* send_reply(socket) */
    }

    state->to = malloc(sizeof(char) * MAX_SIZE);
    strncpy(state->to , command+10, strlen(command)-10-1);

    // CHECK IF USER EXISTS ETC
    printf(">> Received to '%s'\n", state->to);
    assert(strlen(state->to) != 0);


    send_reply(socket, 250, "OK");
}

void handle_cmd_data(int socket, char *command, State *state) {

    char *temp = malloc(sizeof(char) * BUF_SIZE);
    int has_ended = 0;
    while ( !has_ended ) {
        memset(temp, 0, BUF_SIZE);
        recv(socket, temp, BUF_SIZE,0);
        printf("DATA recv: '%s'\n", temp);
        // append to str
        // compare decide if more

    }

}

void handle_cmd_rset(int socket, char *command, State *state) {
    memset(state, 0 , sizeof(State) );
    send_reply(socket, 250, HOSTNAME);
}
void handle_cmd_quit(int socket, char*command, State *state) {
    send_reply(socket, 221, HOSTNAME " Service closing transmission channel");
    shutdown(socket, 2);
    exit(0); // todo: find cleaner way
}

void handle_client(int socket) {
    printf("Handling client with socket %d\n", socket);

    char *command = malloc(sizeof(char) * BUF_SIZE);

    State *state = malloc(sizeof(State));
    // what all state do need to keep?

    while(1) {
        memset(command, 0, BUF_SIZE);
        int r = recv(socket, command, BUF_SIZE, 0);
        if ( r == -1) continue;
        if ( r == 0) {
            printf("Client closed connection");
            return;
        }
        printf("CMD: '%s'\n", command);
        if ( starts_with(command, "HELO") ) {
            handle_cmd_helo(socket, command,  state);
        } else if ( starts_with(command, "QUIT") ) {
            handle_cmd_quit(socket, command, state);
        } else if ( starts_with(command, "MAIL") ) {
            handle_cmd_mail(socket,command, state);
        } else if ( starts_with(command, "RCPT") ) {
            handle_cmd_rcpt(socket, command, state);
        } else if ( starts_with(command, "DATA") ) {
            handle_cmd_data(socket,command, state);
        } else if ( starts_with(command, "RSET" ) ) {
            handle_cmd_rset(socket, command, state);
        } else if ( starts_with(command, "NOOP" ) ) {
            send_reply(socket, 250, "NOOP");
        } else {
            send_reply(socket, 502, "not implemented");
            // reply with 505 command not implemented
        }
    }

}

int main (int argc, char *argv[]) {
    if ( argc < 2 ) {
        printf("./smtpmail PORT\n");
        return 1;
    }

    load_usersfile();
    create_folders();


    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if ( setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt() failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address));
    if ( success != 0) {
        printf("bind() failed");
        return 1;
    }
    listen(server_sock, BACKLOG);
    printf("Listening on port %d\n", atoi(argv[1]));

    while(1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if ( client_sock == -1 ) continue;
        if ( !fork() ) {
            close(server_sock);
            handle_client(client_sock);
        }
        close(client_sock);
    }
    close(server_sock);

    return 0;
}
