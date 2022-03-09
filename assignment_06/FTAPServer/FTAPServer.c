#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 4035
#define BUF_SIZE 1024
#define BACKLOG 5
#define MAX_SIZE 100
#define USER_FILE "../logincred.txt"

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
    while ( t != NULL && t->next != NULL) {
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
    while ( t != NULL && t->next != NULL) {
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

int load_usersfile() {
    FILE *fp = fopen(USER_FILE, "r");
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
        password[strlen(password)-1] = '\0';
        strcpy(password, line+i+1);
        add_user(username, password);
        /* printf("%s:%s\n", username, password); */
    }

    return 0;
}

typedef struct _packet {
    int type;
    int size;
    char data[BUF_SIZE];
} Packet;

/*
 * type_1 -> file data
 * type_2 -> file end marker
 * while ( ! type2) receive data and append to file
 */

void handle_store_file() {
    // Sends the file
    // Sends end marker
}
void handle_get_file(){
    // receive file until you get end marker
}

void handle_create_file(){
    // touch the file
}

void handle_list_dir() {

}


void handle_client(int client_socket) {
    while (1) {
        /* int r = recv(client_socket, &p, sizeof(Packet), 0); */
        int r = 99;
        if ( r == -1) {
            continue;
        }
        if ( r == 0) {
            printf("Client closed connection");
            return;
        }

        char * message = "";

        if ( strncmp("START", message, 5) == 0 ) {
            // Directly reply with 200 Connection is setup
        } else if ( strncmp("USERN", message, 5) == 0) {
            // - Validate the username 301 if invaid
            // - store, and reply 300
        } else if ( strncmp("PASSWD", message, 6) == 0) {
            // - 305 if success , else 310

        } else if ( strncmp("StoreFile", message ,9) == 0) {

        } else if ( strncmp("GetFile", message, 7) == 0 ) {

        } else if ( strncmp("CreateFile", message, 10) == 0) {
            // Do touch on that filename
        } else if ( strncmp("ListDir", message, 7) == 0) {
            // use ls as shell
        } else if ( strncmp("QUIT", message, 4) == 0) {

        } else {

        }
    }
}

int main ()
{
    load_usersfile();
    /* print_users(); */

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


    printf("Waiting for connections\n");
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
