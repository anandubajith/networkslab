#include <dirent.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT 4035
#define BUF_SIZE 1024
#define BACKLOG 5
#define MAX_SIZE 100
#define PACKET_SIZE 1024
#define USER_FILE "../logincred.txt"

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
    while (t != NULL && t->next != NULL) {
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
    while (t != NULL && t->next != NULL) {
        if (strcmp(t->name, username) == 0) {
            if (strcmp(t->password, password) == 0) {
                // successful authentication
                return 0;
            }
            // incorrect password
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

int load_usersfile() {
    FILE *fp = fopen(USER_FILE, "r");
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
        while (line[i] != EOF && line[i] != ',') {
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
        /* printf("%s:%s\n", username, password); */
    }

    return 0;
}

typedef struct _packet {
    int code;
    int size;
    char data[PACKET_SIZE];
} Packet;

void handle_store_file(int socket, char *filename) {
    Packet *p = malloc(sizeof(Packet));
    memset(p, 0, sizeof(*p));

    if (access(filename, F_OK) == 0) {
        // Trying to store a file which already exists on server
        p->code = 611;
        sprintf(p->data, "File %s', already exists", filename);
        printf("%s\n", p->data);
        send(socket, p, sizeof(*p), 0);
        return;
    }
    // send ready packet?
    p->code = 600;
    strcpy(p->data, "Sending file");
    send(socket, p, sizeof(*p), 0);

    FILE *fp = fopen(filename, "wb");

    int recv_size = recv(socket, p, sizeof(*p), 0);
    if (recv_size <= 0) {
        printf("Server closed connection\n");
        return;
    }

    /* printf("Received packet with code = %d\n", p->code); */
    if (p->code != 601) {
        printf("Invalid file info packet\n");
        return;
    }
    // read file_info packet and
    int total_size = 0;
    sscanf(p->data, "%d", &total_size);

    while (1) {
        memset(p, 0, sizeof(*p));
        int recv_size = recv(socket, p, sizeof(*p), 0);
        /* printf("recv_size = %d\n" , recv_size); */
        if (recv_size <= 0) {
            printf("Server closed connection\n");
            return;
        }
        /* printf("%s", p->data); */
        fwrite(p->data, sizeof(char), p->size, fp);
        /* printf("Received packet with code = %d size = %d \n", p->code, p->size);
        */
        if (p->code == 603) {
            break;
        }
    }

    fclose(fp);
    free(p);
    printf("Uploading '%s' complete\n", filename);
}

void handle_get_file(int socket, char *filename) {
    Packet *p = malloc(sizeof(Packet));
    memset(p, 0, sizeof(*p));

    if (access(filename, F_OK) != 0) {
        // send file does not exist packet
        p->code = 610;
        sprintf(p->data, "File '%s' does not exist", filename);
        printf("%s\n", p->data);
        send(socket, p, sizeof(*p), 0);
        return;
    }

    FILE *fp = fopen(filename, "rb");

    // send FileInfo packet99069
    fseek(fp, 0L, SEEK_END);
    p->code = 601;
    int file_size = ftell(fp);

    sprintf(p->data, "%d", file_size);
    p->size = strlen(p->data);
    /* printf("Total File Size = %s\n", p->data); */
    fseek(fp, 0L, SEEK_SET);
    send(socket, p, sizeof(*p), 0);

    memset(p, 0, sizeof(*p));

    int count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    while (count) {
        // to decide FileData , or FileEnd
        p->code = ftell(fp) == file_size ? 603 : 602;
        p->size = count;
        send(socket, p, sizeof(*p), 0);

        /*
         * it seems the issue is window size, and breaking up of packets
         */
        usleep(100);
        memset(p, 0, sizeof(*p));
        count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    }
    fclose(fp);
    free(p);
    printf("Downloading '%s' complete\n", filename);
}

void handle_create_file(int socket, char *filename) {
    Packet *p = malloc(sizeof(Packet));

    if (access(filename, F_OK) == 0) {
        // Trying to store a file which already exists on server
        p->code = 611;
        sprintf(p->data, "File '%s' already exists", filename);
        printf("%s\n", p->data);
        send(socket, p, sizeof(*p), 0);
        return;
    }
    FILE *fp = fopen(filename, "w");

    p->code = 604;
    memset(p->data, 0, PACKET_SIZE);
    sprintf(p->data, "File '%s' Created successfully", filename);
    send(socket, p, sizeof(*p), 0);
    printf("%s\n", p->data);
    fclose(fp);
    free(p);
}

void handle_list_dir(int socket) {
    printf("Sending directory listing\n");
    Packet *p = malloc(sizeof(Packet));
    memset(p->data, 0, PACKET_SIZE);
    p->code = 700;
    DIR *d = opendir(".");
    if (d) {
        struct dirent *dir;
        int file_count = 0;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                file_count++;
                sprintf(p->data + strlen(p->data), "%s\n", dir->d_name);
            }
        }
        p->data[strlen(p->data) -1] = '\0';
        closedir(d);
        p->size = file_count;
        send(socket, p, sizeof(*p), 0);
    } else {
        // todo: send error packet
        memset(p->data, 0, PACKET_SIZE);
        p->code = 710;
        send(socket, p, sizeof(*p), 0);
    }

    free(p);
}

void handle_close(int socket) {
    Packet *p = malloc(sizeof(Packet));
    memset(p->data, 0, PACKET_SIZE);
    strcpy(p->data, "GoodBye");
    p->code = 495;
    send(socket, p, sizeof(*p), 0);
    shutdown(socket, 2);
}

void handle_client(int client_socket) {
    printf("Received client\n");
    Packet *p = malloc(sizeof(Packet));

    char *message = malloc(sizeof(char) * BUF_SIZE);
    char *username = malloc(sizeof(char) * BUF_SIZE);

    int status = 0;
    /*
     * 0 => waiting for username
     * 1 => waiting for password
     * 2 => Authenticated
     */

    while (1) {
        memset(message, 0, BUF_SIZE);
        int r = recv(client_socket, message, BUF_SIZE, 0);
        if (r == -1) {
            continue;
        }
        if (r == 0) {
            printf("Client closed connection");
            return;
        }

        memset(p, 0, sizeof(*p));

        if (strncmp("START", message, 5) == 0) {
            p->code = 200;
            strcpy(p->data, "OK Connection is setup");
            send(client_socket, p, sizeof(*p), 0);
        } else if (strncmp("USERN", message, 5) == 0) {
            if ( status > 1) {
                p->code = 332;
                strcpy(p->data, "Username already provided");
            } else if (check_username(message + 6) == 0) {
                // valid username
                p->code = 300;
                strcpy(p->data, "Correct Username; Need password");
                memset(username, 0, BUF_SIZE);
                strcpy(username, message + 6);
                status = 1;
            } else {
                p->code = 301;
                strcpy(p->data, "Incorrect Username");
            }
            send(client_socket, p, sizeof(*p), 0);
        } else if (strncmp("PASSWD", message, 6) == 0) {
            if ( status == 2 ) {
                p->code = 305;
                strcpy(p->data, "Already authenticated");
            } if (strlen(username) == 0) {
                p->code = 302;
                strcpy(p->data, "Missing username;provide username with USERN");
            } else if (check_password(username, message + 7) == 0) {
                p->code = 305;
                sprintf(p->data, "User Authenticated with password\nWelcome, %s!", username);
                printf("User '%s' Authenticated\n", username);
                status = 2;
            } else {
                p->code = 310;
                strcpy(p->data, "Incorrect password");
            }
            send(client_socket, p, sizeof(*p), 0);
        } else if (strncmp("QUIT", message, 4) == 0) {
            printf("User '%s' Disconnected\n", username);
            handle_close(client_socket);
        } else if (status != 2) {
            p->code = 333;
            strcpy(p->data, "Authentication required");
            send(client_socket, p, sizeof(*p), 0);
        } else if (strncmp("StoreFile", message, 9) == 0) {
            handle_store_file(client_socket, message + 10);
        } else if (strncmp("GetFile", message, 7) == 0) {
            handle_get_file(client_socket, message + 8);
        } else if (strncmp("CreateFile", message, 10) == 0) {
            handle_create_file(client_socket, message + 11);
        } else if (strncmp("ListDir", message, 7) == 0) {
            handle_list_dir(client_socket);
        } else {
            p->code = 505;
            strcpy(p->data, "Command not supported");
            send(client_socket, p, sizeof(*p), 0);
        }
    }
}

int main() {
    load_usersfile();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (success != 0) {
        printf("Bind failed");
        exit(-1);
    }

    listen(server_sock, BACKLOG);

    printf("Waiting for connections\n");
    while (1) {
        int client_socket = accept(server_sock, NULL, NULL);
        if (client_socket == -1) {
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
