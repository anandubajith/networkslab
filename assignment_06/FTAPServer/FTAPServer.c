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
    int code;
    int size;
    char data[504];
} Packet;

void handle_get_file(int socket, char* filename) {
    Packet *p = malloc(sizeof(Packet));
    FILE* fp = fopen(filename, "wb");

    memset(p, 0, sizeof(*p));
    int recv_size = recv(socket, p, sizeof(*p), 0);
    if ( recv_size <= 0 ) {
        printf("Server closed connection\n");
        return;
    }

        printf("Received packet with code = %d\n", p->code);
    if ( p->code != 601) {
        printf("INvalid FileInfoPacket\n");
        return;
    }
    // read file_info packet and
    int total_size = 0;
    sscanf(p->data, "%d", &total_size);



    while (1) {
        memset(p, 0, sizeof(*p));
        int recv_size = recv(socket, p, sizeof(*p), 0);
        printf("recv_size = %d\n" , recv_size);
        if ( recv_size <= 0 ) {
            printf("Server closed connection");
            return;
        }
        /* printf("%s", p->data); */
        fwrite(p->data, sizeof(char), p->size, fp);
        // todo: print progress

        printf("Received packet with code = %d size = %d \n", p->code, p->size);
        if ( p->code == 603) {
            break;
        }
    }

    fclose(fp);
    recv_size = recv(socket, p, sizeof(*p), 0);
    if ( recv_size <= 0 ) {
        printf("Server closed connection\n");
        return;
    }

    free(p);
}

void handle_send_file(int socket, char *filename) {

}

void handle_create_file(int socket, char *filename){
    // touch the file
}

void handle_list_dir(int socket) {

}

void handle_close(int socket) {

}

void handle_client(int client_socket) {
    printf("Received client\n");
    Packet *p = malloc(sizeof(Packet));
    char* message = malloc(sizeof(char) * BUF_SIZE);
    char* username = malloc(sizeof(char) * BUF_SIZE);

    while (1) {
        int r = recv(client_socket, message, BUF_SIZE, 0);
        if ( r == -1) {
            continue;
        }
        if ( r == 0) {
            printf("Client closed connection");
            return;
        }

        memset(p, 0, sizeof(*p));
        if ( strncmp("START", message, 5) == 0 ) {
            // Directly reply with 200 Connection is setup
            p->code = 200;
            strcpy(p->data, "OK Connection is setup");
            send(client_socket, p, sizeof(*p), 0);
        } else if ( strncmp("USERN", message, 5) == 0) {
            if ( check_username(message+6)  == 0 ) {
                // valid username
                p->code = 300;
                strcpy(p->data, "Correct Username; Need password");
                strcpy(username, message+5);
            } else {
                p->code = 301;
                strcpy(p->data, "Incorrect Username");
            }
            send(client_socket, p, sizeof(*p), 0);
        } else if ( strncmp("PASSWD", message, 6) == 0) {
            if ( strlen(username) == 0 ) {
                // todo: handle username not set case
            }
            if ( check_password(username, message + 7) == 0) {
                p->code = 310;
                strcpy(p->data, "User authenticated with password");
                // todo: welcome username
            } else {
                p->code = 310;
                strcpy(p->data, "Incorrect password");
            }
            send(client_socket, p, sizeof(*p), 0);
        } else if ( strncmp("StoreFile", message ,9) == 0) {
            handle_get_file(client_socket,message+10);
        } else if ( strncmp("GetFile", message, 7) == 0 ) {
            // todo: extract the filename
            /* handle_send_file(); */
        } else if ( strncmp("CreateFile", message, 10) == 0) {
            handle_create_file(client_socket, message+11);
        } else if ( strncmp("ListDir", message, 7) == 0) {
            handle_list_dir(client_socket);
        } else if ( strncmp("QUIT", message, 4) == 0) {
            handle_close(client_socket);
        } else {
            p->code = 505;
            strcpy(p->data, "Command not supported");
            send(client_socket, p, sizeof(*p), 0);
        }
    }
}

int main ()
{
    load_usersfile();

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
        /* if (!fork()) { */
            /* close(server_sock); */
            handle_client(client_socket);
        /* } */
        /* close(client_socket); */
    }

    close(server_sock);
    return 0;
}
