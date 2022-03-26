#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <linux/tcp.h>
#include <unistd.h>

#define BACKLOG 5
#define MAX_SIZE 100
#define BUF_SIZE 512
#define HOSTNAME "localhost"

typedef struct _user {
    char *name;
    char *password;
    struct _user *next;
} User;

User *usersHead = NULL;

int add_user(char *username, char *password) {
    User *u = malloc(sizeof(User));
    u->name = malloc(sizeof(char) * MAX_SIZE);
    strcpy(u->name, username);
    u->password = malloc(sizeof(char) * MAX_SIZE);
    strcpy(u->password, password);

    if (usersHead == NULL) {
        usersHead = u;
        return 0;
    }

    User *t = usersHead;
    while (t->next != NULL) {
        if (strcmp(t->name, username) == 0) {
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
    while (t != NULL) {
        if (strcmp(t->name, username) == 0) {
            // username already exists
            return 0;
        }
        t = t->next;
    }
    return 1;
}

int check_password(char *username, char *password) {
    User *t = usersHead;
    while (t != NULL) {
        if (strcmp(t->name, username) == 0) {
            if (strcmp(t->password, password) == 0) {
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
    while (t != NULL) {
        printf("username:%s password:%s\n", t->name, t->password);
        t = t->next;
    }
}

void create_folders() {
    struct stat st = {0};
    User *t = usersHead;
    char *buffer = malloc(sizeof(char) * MAX_SIZE);
    while (t != NULL) {
        memset(buffer, 0, MAX_SIZE);
        sprintf(buffer, "./%s", t->name);
        if (stat(buffer, &st) == -1) {
            mkdir(buffer, 0700);
        }
        t = t->next;
    }
}
int load_usersfile() {
    FILE *fp = fopen("./userlogincred.txt", "r");
    if (fp == NULL)
        return -1;

    char line[2 * MAX_SIZE + 1]; // username + , + password
    char username[MAX_SIZE];
    char password[MAX_SIZE];

    while (fgets(line, sizeof(line), fp)) {
        // split line by ,
        memset(username, 0, MAX_SIZE);
        memset(password, 0, MAX_SIZE);
        int i = 0;
        while (line[i] != EOF && line[i] != ' ') {
            i++;
        }
        if (line[i] == EOF) {
            fprintf(stderr, "Malformed line: %s\n", line);
            continue;
        }
        strncpy(username, line, i);
        strcpy(password, line + i + 1);
        password[strlen(password) - 1] = '\0';
        add_user(username, password);
        /* printf("'%s':'%s'\n", username, password); */
    }
    return 0;
}

int starts_with(char *string, char *marker) {
    assert(string != NULL);
    assert(marker != NULL);
    return strncmp(string, marker, strlen(marker)) == 0;
}

void send_reply(int socket, int code, char *message) {
    char *temp = malloc(sizeof(char) * BUF_SIZE);
    memset(temp, 0, BUF_SIZE);
    sprintf(temp, "%d %s\n", code, message);
    /* printf("sending reply '%s'\n", temp); */
    send(socket, temp, strlen(temp), 0);
    free(temp);
}

char *get_current_time_str() {
    time_t t; // not a primitive datatype
    time(&t);
    return ctime(&t);
}

typedef struct _state {
    char *from;
    char *from_user;
    char *from_host;
    char *to;
    char *to_user;
    char *to_host;
    char *body;
    int current;
} State;

int validate_address(char *address, char **user, char **host) {
    // split address into 2

    if (strlen(address) == 0)
        return 1;

    unsigned long pos = 0;
    while (address[pos] != 0 && address[pos] != '@')
        pos++;

    if (pos == strlen(address))
        return 1;

    *user = malloc(sizeof(char) * pos + 1);
    *host = malloc(sizeof(char) * strlen(address + pos) + 1);

    strncpy(*user, address, pos);
    strcpy(*host, address + pos + 1);

    if (strlen(*host) == 0 || strlen(*user) == 0)
        return 1;

    /* printf("'%s' '%s'\n\n", *user, *host); */

    return 0;
}

void process_state(State *state) {
    // we've already validated the user
    char *path = malloc(sizeof(char) * MAX_SIZE * 2);
    sprintf(path, "./%s/mymailbox", state->to_user);
    FILE *fp = fopen(path, "a");

    fprintf(fp, "from: %s@%s\n", state->from_user, state->from_host);
    fprintf(fp, "to: %s@%s\n", state->to_user, state->to_host);
    fprintf(fp, "received: %s\n", get_current_time_str());
    fprintf(fp, "%s", state->body);
    fprintf(fp, "\n.\n");
    fclose(fp);
}

void handle_cmd_helo(int socket, State *state) {
    // clear state
    memset(state, 0, sizeof(State));
    send_reply(socket, 250, HOSTNAME);
}
void handle_cmd_mail(int socket, char *command, State *state) {
    assert(state != NULL);
    if (starts_with("MAIL FROM:", command) == 1) {
        send_reply(socket, 501, "Syntax error in parameters or arguments");
        return;
    }

    state->from = malloc(sizeof(char) * MAX_SIZE);
    strncpy(state->from, command + 11, strlen(command + 11) - 1);

    if (validate_address(state->from, &state->from_user, &state->from_host) != 0) {
        send_reply(socket, 501, "Syntax error in parameters or arguments");
        return;
    }

    if (strncmp(state->from_host, HOSTNAME, 9) != 0) {
        send_reply(socket, 551, "User not local;");
        return;
    }

    if (check_username(state->from_user) != 0) {
        send_reply(socket, 550, "Requested action not taken: mailbox unavailable");
        return;
    }

    printf(">> Received from '%s'\n", state->from);
    assert(strlen(state->from) != 0);

    send_reply(socket, 250, "OK");
}
void handle_cmd_rcpt(int socket, char *command, State *state) {
    assert(state != NULL);
    if (starts_with("RCPT TO:", command) == 1) {
        send_reply(socket, 501, "Syntax error in parameters or arguments");
        return;
    }

    state->to = malloc(sizeof(char) * MAX_SIZE);
    strncpy(state->to, command + 9, strlen(command + 9) - 1);

    if (validate_address(state->to, &state->to_user, &state->to_host) != 0) {
        send_reply(socket, 501, "Syntax error in parameters or arguments");
        return;
    }

    if (strncmp(state->to_host, HOSTNAME, 9) != 0) {
        send_reply(socket, 551, "User not local;");
        return;
    }

    if (check_username(state->to_user) != 0) {
        send_reply(socket, 550, "Requested action not taken: mailbox unavailable");
        return;
    }

    printf(">> Received to '%s'\n", state->to);
    assert(strlen(state->to) != 0);

    send_reply(socket, 250, "OK");
}

void handle_cmd_data(int socket, State *state) {

    // todo: refactor this
    state->body = malloc(sizeof(char) * BUF_SIZE * 1000);
    memset(state->body, 0, BUF_SIZE *1000);
    char *temp = malloc(sizeof(char) * BUF_SIZE);

    while (1) {
        memset(temp, 0, BUF_SIZE);
        int r = recv(socket, temp, BUF_SIZE, 0);
        if ( r <= 0) break;
        printf("DATA recv: '%s'\n", temp);
        if (strlen(temp) == 1 && temp[0] == '.') {
            printf("Received end marker\n");
            break;
        }
        if ( strlen(state->body) > 0 )
            strcat(state->body, "\n");
        strcat(state->body, temp);
    }

    process_state(state);
    send_reply(socket, 250, "OK");
}

void handle_cmd_rset(int socket, State *state) {
    memset(state, 0, sizeof(State));
    send_reply(socket, 250, HOSTNAME);
}
void handle_cmd_quit(int socket, State *state) {
    free(state);
    send_reply(socket, 221, HOSTNAME " Service closing transmission channel");
    shutdown(socket, 2);
    exit(0); // todo: find cleaner way
}

void handle_client(int socket) {
    printf("Handling client with socket %d\n", socket);

    setsockopt(socket, SOL_SOCKET,TCP_NODELAY , (char *) &(int){1}, sizeof(int));

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    char *command = malloc(sizeof(char) * BUF_SIZE);

    State *state = malloc(sizeof(State));

    while (1) {
        memset(command, 0, BUF_SIZE);
        int r = recv(socket, command, BUF_SIZE, 0);
        if (r == -1)
            continue;
        if (r == 0) {
            printf("Client closed connection");
            return;
        }
        printf("CMD: '%s'\n", command);
        if (starts_with(command, "HELO")) {
            handle_cmd_helo(socket, state);
        } else if (starts_with(command, "QUIT")) {
            handle_cmd_quit(socket, state);
        } else if (starts_with(command, "MAIL")) {
            handle_cmd_mail(socket, command, state);
        } else if (starts_with(command, "RCPT")) {
            handle_cmd_rcpt(socket, command, state);
        } else if (starts_with(command, "DATA")) {
            handle_cmd_data(socket, state);
        } else if (starts_with(command, "RSET")) {
            handle_cmd_rset(socket, state);
        } else if (starts_with(command, "NOOP")) {
            send_reply(socket, 250, "NOOP");
        } else {
            send_reply(socket, 502, "not implemented");
            // reply with 505 command not implemented
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("./smtpmail PORT\n");
        return 1;
    }

    load_usersfile();
    create_folders();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1},
                sizeof(int)) < 0)
        printf("setsockopt() failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr *)&server_address,
            sizeof(server_address));
    if (success != 0) {
        printf("bind() failed");
        return 1;
    }
    listen(server_sock, BACKLOG);
    printf("Listening on port %d\n", atoi(argv[1]));

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock == -1)
            continue;
        if (!fork()) {
            close(server_sock);
            handle_client(client_sock);
        }
        close(client_sock);
    }
    close(server_sock);

    return 0;
}
