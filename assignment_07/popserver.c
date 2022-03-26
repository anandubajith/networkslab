#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/tcp.h>
#include <unistd.h>

#define BACKLOG 5
#define MAX_SIZE 100
#define BUF_SIZE 512

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
        if (username != NULL && strcmp(t->name, username) == 0) {
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
        if (username != NULL && strcmp(t->name, username) == 0) {
            if (password != NULL && strcmp(t->password, password) == 0) {
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

void free_users() {
    while (usersHead != NULL) {
        User* u = usersHead;
        usersHead = u->next;
        free(u->name);
        free(u->password);
        free(u);
    }
}

void print_users() {
    User *t = usersHead;
    while (t != NULL) {
        printf("username:%s password:%s\n", t->name, t->password);
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
typedef struct _mail {
    char *from;
    char *to;
    char *subject;
    char *received_time;
    char *body;
    char *body_raw;
    int index;
    int size;
    int is_deleted;
    struct _mail *next;
    struct _mail *prev;
} Mail;

void print_mails(Mail *mail) {
    while (mail != NULL) {
        printf("---\n");
        printf("index: %d deleted: %d\n", mail->index, mail->is_deleted);
        printf("to: '%s'\n", mail->to);
        printf("from: '%s'\n", mail->from);
        printf("received_time: '%s'\n", mail->received_time);
        printf("subject: '%s'\n", mail->subject);
        printf("DATA:\n");
        printf("%s", mail->body);
        printf("DATA_RAW:\n");
        printf("%s", mail->body_raw);
        printf("---\n");
        mail = mail->next;
    }
}

char *get_mailbox_path(char *username) {
    char *path = malloc(sizeof(char) * MAX_SIZE * 2);
    sprintf(path, "./%s/mymailbox", username);
    return path;
}

void update_emails(char *username, Mail *head) {
    char *path = get_mailbox_path(username);
    FILE *fp = fopen(path, "w");
    Mail *iter = head;
    while (iter != NULL) {
        if (iter->is_deleted != 1) {
            fprintf(fp, "from: %s\n", iter->from);
            fprintf(fp, "to: %s\n", iter->to);
            fprintf(fp, "received: %s\n", iter->received_time);
            fprintf(fp, "subject: %s\n", iter->subject);
            fprintf(fp, "%s", iter->body_raw);
            fprintf(fp, ".\n");
        }
        iter = iter->next;
    }
    free(path);
    fclose(fp);
}

Mail *load_messages(char *username) {
    char *path = get_mailbox_path(username);
    FILE *fp = fopen(path, "r");

    if ( fp == NULL ) {
        return NULL;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int count = 0;
    int state = 0;
    Mail *mailHead = NULL;

    Mail *current_mail = malloc(sizeof(Mail));
    int current_size = 0;
    memset(current_mail, 0, sizeof(Mail));

    while ((read = getline(&line, &len, fp)) != -1) {
        /* printf("'%s'\n",line); */
        current_size += read;
        if (state == 0) {
            if (strncmp("from: ", line, 6) == 0) {
                int len = strlen(line + 6);
                current_mail->from = malloc(sizeof(char) * len);
                memset(current_mail->from, 0, len);
                strncpy(current_mail->from, line + 6, len - 1);
            } else if (strncmp("to: ", line, 4) == 0) {
                int len = strlen(line + 4);
                current_mail->to = malloc(sizeof(char) * len);
                memset(current_mail->to, 0, len);
                strncpy(current_mail->to, line + 4, len - 1);
            } else if (strncmp("received: ", line, 10) == 0) {
                int len = strlen(line + 10);
                current_mail->received_time = malloc(sizeof(char) * len);
                memset(current_mail->received_time, 0, len);
                strncpy(current_mail->received_time, line + 10, len - 1);
            } else if (strncmp("subject: ", line, 9) == 0) {
                int len = strlen(line + 9);
                current_mail->subject = malloc(sizeof(char) * len);
                memset(current_mail->subject, 0, len);
                strncpy(current_mail->subject, line + 9, len - 1);
                state = 1;
            } else {
                // data corruption?
            }
        } else if (state == 1) {
            // append to data follwing transparancy procedure
            if (current_mail->body == NULL) {
                current_mail->body = malloc(sizeof(char) * strlen(line) + 1);
                current_mail->body_raw = malloc(sizeof(char) * strlen(line) + 1);
                memset(current_mail->body, 0, strlen(line) + 1);
                memset(current_mail->body_raw, 0, strlen(line) + 1);
            } else {
                current_mail->body = realloc( current_mail->body, strlen(current_mail->body) + strlen(line) + 1);
                current_mail->body_raw = realloc(current_mail->body_raw, strlen(current_mail->body_raw) + strlen(line) + 1);
            }
            if (strncmp(".", line, 1) == 0 && strlen(line) == 2) {
                state = 0;
                current_mail->next = mailHead;
                if (mailHead != NULL)
                    mailHead->prev = current_mail;
                current_mail->index = ++count;
                current_mail->size = current_size;
                mailHead = current_mail;
                current_mail = malloc(sizeof(Mail));
                current_size = 0;
                memset(current_mail, 0, sizeof(Mail));
            } else {
                /* printf("Cattign :'%s' with '%s'", mail->body, line); */
                strcat(current_mail->body, line + (line[0] == '.' ? 1 : 0));
                strcat(current_mail->body_raw, line);
            }
        } else {
            // data corruption?
        }
    }

    if (line)
        free(line);

    fclose(fp);
    free(path);
    return mailHead;
}

void handle_cmd_stat(int socket, Mail *mailHead) {
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    Mail *iter = mailHead;

    int count = 0;
    int total_size = 0;
    while (iter != NULL) {
        count++;
        total_size += iter->size;
        iter = iter->next;
    }

    sprintf(buffer, "+OK %d %d\n", count, total_size);
    send(socket, buffer, strlen(buffer), 0);
    free(buffer);
}

void handle_cmd_list(int socket, Mail *mailHead) {
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);

    Mail *iter = mailHead;

    int count = 0;
    int total_size = 0;
    while (iter != NULL) {
        count++;
        if (iter->is_deleted != 1)
            total_size += iter->size;
        iter = iter->next;
    }

    sprintf(buffer, "+OK %d messages (%d octects)\n", count, total_size);
    send(socket, buffer, strlen(buffer), 0);

    iter = mailHead;
    while (iter != NULL) {
        if (iter->is_deleted != 1) {
            memset(buffer, 0, BUF_SIZE);
            sprintf(buffer, "%d %d\n", iter->index, iter->size);
            send(socket, buffer, strlen(buffer), 0);
        }
        iter = iter->next;
    }

    free(buffer);
}

void handle_cmd_retr(int socket, int index, Mail *mailHead) {
    printf("retreiving %d email\n", index);
    Mail *mail = mailHead;
    while (mail != NULL) {
        if (mail->index == index && mail->is_deleted == 0) {
            break;
        }
        mail = mail->next;
    }

    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    if (mail == NULL) {
        // invalid index
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "-ERR Invalid index");
        send(socket, buffer, strlen(buffer), 0);
        return;
    }

    printf("foudn the index %d\n", mail->index);

    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "+OK %d octets\n", mail->size);
    send(socket, buffer, strlen(buffer), 0);
    usleep(1000);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "from: %s\n", mail->from);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "to: %s\n", mail->to);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "received: %s\n", mail->received_time);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "subject: %s\n\n", mail->subject);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "%s", mail->body);
    send(socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "\n\n");
    send(socket, buffer, strlen(buffer), 0);

    free(buffer);
}

void handle_cmd_dele(int socket, int index, Mail *mailHead) {

    Mail *mail = mailHead;
    while (mail != NULL) {
        if (mail->index == index && mail->is_deleted == 0) {
            break;
        }
        mail = mail->next;
    }

    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    if (mail == NULL) {
        // invalid index
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "-ERR no such message");
        send(socket, buffer, strlen(buffer), 0);
        return;
    }

    if (mail->is_deleted == 1) {
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "-ERR message %d already deleted", mail->index);
        send(socket, buffer, strlen(buffer), 0);
        return;
    }

    mail->is_deleted = 1;
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "+OK message %d deleted", mail->index);
    send(socket, buffer, strlen(buffer), 0);
}

void handle_cmd_rset(int socket, char *username, Mail **mailHead) {
    // free everything
    Mail *prev = NULL;
    Mail *iter = *mailHead;
    while (iter != NULL) {
        free(iter->body_raw);
        free(iter->from);
        free(iter->to);
        free(iter->subject);
        free(iter->received_time);
        free(iter->body_raw);
        prev = iter;
        iter = iter->next;
        free(prev);
    }

    *mailHead = load_messages(username);

    iter = *mailHead;
    int count = 0;
    int total_size = 0;
    while (iter != NULL) {
        count++;
        if (iter->is_deleted != 1)
            total_size += iter->size;
        iter = iter->next;
    }

    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "+OK maildrop has %d messages (%d octets)", count,
            total_size);
    send(socket, buffer, strlen(buffer), 0);
    free(buffer);
}

void handle_cmd_top(int socket, char *command, Mail *head) {
    printf(">> handling command : '%s'\n", command);
    int message_index = 0;
    int count = 0;
    if ( sscanf(command +3, "%d %d", &message_index, &count) != 2 ) {
        // invalid format
        send(socket, "-ERR Invalid format", 15, 0);
        return;
    }

    Mail* iter = head;
    while (iter != NULL) {
        if ( iter->index == message_index && iter->is_deleted != 1) {
            break;
        }
        iter = iter->next;
    }
    if ( iter == NULL) {
        send(socket, "-ERR no such message", 20, 0);
        return;
    }

    char *buffer = malloc(sizeof(char) * BUF_SIZE *1000);
    // send OK message
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "+OK top of message follows\r\n");
    send(socket, buffer, strlen(buffer), 0);

    if (count > 0) {
        memset(buffer,0, BUF_SIZE);
        sprintf(buffer, "from: %s\r\n", iter->from);
        send(socket, buffer, strlen(buffer), 0);
        count--;
    }
    if (count > 0) {
        memset(buffer,0, BUF_SIZE);
        sprintf(buffer, "to: %s\r\n", iter->to);
        send(socket, buffer, strlen(buffer), 0);
        count--;
    }
    if (count > 0) {
        memset(buffer,0, BUF_SIZE);
        sprintf(buffer, "received: %s\r\n", iter->received_time);
        send(socket, buffer, strlen(buffer), 0);
        count--;
    }
    if (count > 0) {
        memset(buffer,0, BUF_SIZE);
        sprintf(buffer, "subject: %s\r\n", iter->subject);
        send(socket, buffer, strlen(buffer), 0);
        count--;
    }

    if ( count > 0 ) {
        char *copy = malloc(sizeof(char) * strlen(iter->body) + 1);
        strcpy(copy, iter->body);
        char *part = strtok(copy, "\n");
        while (part != NULL && count > 0) {
            memset(buffer,0, BUF_SIZE);
            sprintf(buffer, "%s\r\n",part);
            send(socket, buffer, strlen(buffer), 0);
            count--;
            part = strtok(NULL, "\n");
        }

        free(copy);
    }

    free(buffer);
}

int starts_with(char *string, char *marker) {
    assert(string != NULL);
    assert(marker != NULL);
    return strncmp(string, marker, strlen(marker)) == 0;
}

void handle_client(int socket) {
    setsockopt(socket, SOL_SOCKET,TCP_NODELAY , (char *) &(int){1}, sizeof(int));
    char *buffer_out = malloc(sizeof(char) * BUF_SIZE);
    char *buffer_in = malloc(sizeof(char) * BUF_SIZE*1000);
    char *work = malloc(sizeof(char) * BUF_SIZE *1000);
    memset(buffer_out, 0, BUF_SIZE);
    strcpy(buffer_out, "+OK POP3 server ready\n");

    send(socket, buffer_out, strlen(buffer_out), 0);

    int state = 0;
    char *username = NULL;
    char *password = NULL;
    Mail *mailHead = NULL;
    /*
     * 0 => Authentiation
     * 1 => Transaction
     */

    while (1) {
        memset(buffer_in, 0, BUF_SIZE*1000);
        int r = recv(socket, buffer_in, BUF_SIZE, 0);
        if (r == -1)
            continue;
        if (r == 0) {
            printf("Client closed connection\n");
            return;
        }
        memset(work, 0, BUF_SIZE*1000);
        strcpy(work, buffer_in);

        char *buffer_split_ptr;

        char *command = strtok_r(work, "\r\n", &buffer_split_ptr);
        while (command != NULL) {
            printf("CMD: '%s'\n", command);
            if (state == 0 && starts_with(command, "USER")) {
                if (username != NULL) {
                    memset(buffer_out, 0, BUF_SIZE);
                    strcpy(buffer_out, "-ERR Already provided username\n");
                    send(socket, buffer_out, strlen(buffer_out), 0);
                    continue;
                }
                username = malloc(sizeof(char) * MAX_SIZE);
                strcpy(username, command + 5);
                /* printf("Extracted username : '%s'\n", username); */
                if (check_username(username) != 0) {
                    memset(buffer_out, 0, BUF_SIZE);
                    strcpy(buffer_out, "-ERR Invalid username\n");
                    send(socket, buffer_out, strlen(buffer_out), 0);
                    free(username);
                    username = NULL;
                    continue;
                }
                memset(buffer_out, 0, BUF_SIZE);
                strcpy(buffer_out, "+OK Username accepted\n");
                send(socket, buffer_out, strlen(buffer_out), 0);
            } else if (state == 0 && starts_with(command, "PASS")) {
                if (password != NULL) {
                    memset(buffer_out, 0, BUF_SIZE);
                    strcpy(buffer_out, "-ERR Already provided password\n");
                    send(socket, buffer_out, strlen(buffer_out), 0);
                    continue;
                }
                password = malloc(sizeof(char) * MAX_SIZE);
                strcpy(password, command + 5);
                /* printf("Extracted password : '%s'", password); */
                if (check_password(username, password) != 0) {
                    memset(buffer_out, 0, BUF_SIZE);
                    strcpy(buffer_out, "-ERR Invalid password\n");
                    send(socket, buffer_out, strlen(buffer_out), 0);
                    free(password);
                    password = NULL;
                    continue;
                }
                memset(buffer_out, 0, BUF_SIZE);
                strcpy(buffer_out, "+OK Auth successful\n");
                send(socket, buffer_out, strlen(buffer_out), 0);
                mailHead = load_messages(username);
                state = 1;
            } else if (starts_with(command, "QUIT")) {
                memset(buffer_out, 0, BUF_SIZE);
                if (state == 1) {
                    // todo: check error?
                    update_emails(username, mailHead);
                    strcpy(buffer_out, "+OK GoodBye");
                } else {
                    strcpy(buffer_out, "+OK GoodBye");
                }
                shutdown(socket, 2);
                break;
            } else if (state == 1 && starts_with(command, "STAT")) {
                handle_cmd_stat(socket, mailHead);
            } else if (state == 1 && starts_with(command, "LIST")) {
                handle_cmd_list(socket, mailHead);
            } else if (state == 1 && starts_with(command, "RETR")) {
                handle_cmd_retr(socket, atoi(command + 4), mailHead);
            } else if (state == 1 && starts_with(command, "DELE")) {
                handle_cmd_dele(socket, atoi(command + 4), mailHead);
            } else if (state == 1 && starts_with(command, "TOP")) {
                handle_cmd_top(socket, command, mailHead);
            } else if (state == 1 && starts_with(command, "NOOP")) {
                memset(buffer_out, 0, BUF_SIZE);
                sprintf(buffer_out, "+OK");
                send(socket, buffer_out, strlen(buffer_out), 0);
            } else if (state == 1 && starts_with(command, "RSET")) {
                handle_cmd_rset(socket, username, &mailHead);
            } else {
                memset(buffer_out, 0, BUF_SIZE);
                strcpy(buffer_out, "-ERR Invalid command\n");
                send(socket, buffer_out, strlen(buffer_out), 0);
            }
            command = strtok_r(NULL, "\r\n", &buffer_split_ptr);

        }
    }
    free(buffer_out);
    free(username);
    free(password);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("./popserver PORT\n");
        return 1;
    }

    load_usersfile();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt() failed");


    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    printf("Pop server\n");
    int success = bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address));
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
        /* if (!fork()) { */
            /* close(server_sock); */
            handle_client(client_sock);
        /* } */
        /* close(client_sock); */
    }
    close(server_sock);

    return 0;
}
