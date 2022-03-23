#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define BACKLOG 5
#define MAX_SIZE 100
#define BUF_SIZE 512

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
        while ( line[i] != EOF && line[i] != ' ' ) {
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
typedef struct _mail {
    char *from;
    char *to;
    char *subject;
    char *received_time;
    char *body;
    char *body_raw;
    int index;
    int is_deleted;
    struct _mail* next;
    struct _mail* prev;
} Mail;

void print_mails(Mail *mail) {
    while ( mail != NULL ) {
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

char* get_mailbox_path(char*username) {
    char *path = malloc(sizeof(char) * MAX_SIZE * 2);
    sprintf(path, "./%s/mymailbox", username);
    return path;
}

void delete_email(char*username, Mail** head, Mail *mail) {
    mail->is_deleted = 1;
}

void update_emails(char*username, Mail**head) {
    char *path = get_mailbox_path(username);
    FILE *fp = fopen(path, "w");
    Mail *iter = *head;
    while( iter != NULL) {
        if ( iter->is_deleted != 1 ) {
            fprintf(fp, "from: %s\n", iter->from);
            fprintf(fp, "to: %s\n", iter->to);
            fprintf(fp, "received: %s\n", iter->received_time );
            fprintf(fp, "subject: %s\n", iter->subject);
            fprintf(fp, "%s", iter->body_raw);
            fprintf(fp, ".\n");
        }
        iter = iter->next;
    }
    free(path);
    fclose(fp);
}

Mail* load_messages(char *username) {
    char *path = get_mailbox_path(username);
    FILE *fp = fopen(path, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int count = 0;
    int state = 0;
    Mail *mailHead = NULL;

    Mail* currentMail = malloc(sizeof(Mail));
    memset(currentMail, 0, sizeof(Mail));

    while ((read = getline(&line, &len, fp)) != -1) {
        /* printf("'%s'\n",line); */
        if ( state == 0) {
            if (strncmp("from: ", line, 6) == 0) {
                int len = strlen(line+6);
                currentMail->from = malloc(sizeof(char) * len );
                memset(currentMail->from, 0, len);
                strncpy(currentMail->from, line+6, len-1);
            } else if ( strncmp("to: ", line, 4) == 0) {
                int len = strlen(line+4);
                currentMail->to = malloc(sizeof(char) * len );
                memset(currentMail->to, 0, len);
                strncpy(currentMail->to, line+4, len-1);
            } else if ( strncmp("received: ", line, 10) == 0) {
                int len = strlen(line+10);
                currentMail->received_time = malloc(sizeof(char) * len );
                memset(currentMail->received_time, 0, len);
                strncpy(currentMail->received_time, line+10, len-1);
            } else if ( strncmp("subject: ", line, 9) == 0) {
                int len = strlen(line+9);
                currentMail->subject = malloc(sizeof(char) * len);
                memset(currentMail->subject, 0, len);
                strncpy(currentMail->subject, line+9, len-1);
                state = 1;
            } else {
                // data corruption?
            }
        } else if ( state == 1) {
            // append to data follwing transparancy procedure
            if ( currentMail->body == NULL ) {
                currentMail->body = malloc(sizeof(char) * strlen(line) + 1);
                currentMail->body_raw = malloc(sizeof(char) * strlen(line) + 1);
                memset(currentMail->body, 0, strlen(line)+1);
                memset(currentMail->body_raw, 0, strlen(line)+1);
            } else {
                currentMail->body = realloc(currentMail->body, strlen(currentMail->body) + strlen(line) +1);
                currentMail->body_raw= realloc(currentMail->body_raw, strlen(currentMail->body_raw) + strlen(line) +1);
            }
            if ( strncmp(".", line, 1) == 0 && strlen(line) == 2) {
                state = 0;
                currentMail->next = mailHead;
                if ( mailHead != NULL) mailHead->prev = currentMail;
                currentMail->index = ++count;
                mailHead = currentMail;
                currentMail = malloc(sizeof(Mail));
                memset(currentMail, 0, sizeof(Mail));
            } else {
                /* printf("Cattign :'%s' with '%s'", mail->body, line); */
                strcat(currentMail->body, line + (line[0] == '.' ? 1 : 0 ));
                strcat(currentMail->body_raw, line);
            }
        } else {
            // data corruption?
        }
    }

    if (line) free(line);

    fclose(fp);
    free(path);
    return mailHead;
}

void handle_cmd_list(int socket, Mail *mailHead) {
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    sprintf(buffer, "+OK %d messages (%d octects)", 0, 123);
    send(socket, buffer, strlen(buffer), 0);
    Mail *iter = mailHead;
    while ( iter != NULL) {
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "%d %d", iter->index, 123);
        send(socket, buffer, strlen(buffer), 0);
        iter = iter->next;
    }

    free(buffer);
}

int starts_with(char *string, char *marker) {
    assert( string != NULL );
    assert( marker != NULL );
    return strncmp(string, marker, strlen(marker)) == 0;
}

void handle_client(int socket) {
    char *command = malloc(sizeof(char) * BUF_SIZE);

    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    strcpy(buffer, "+OK POP3 server ready");

	send(socket, buffer, strlen(buffer), 0);

	int state = 0;
	char* username;
	char* password;
	/*
	 * 0 => Authentiation
	 * 1 => Transaction
	 */

    while(1) {
        memset(command, 0, BUF_SIZE);
        int r = recv(socket, command, BUF_SIZE, 0);
        if ( r == -1) continue;
        if ( r == 0) {
            printf("Client closed connection");
            return;
        }
        printf("CMD: '%s'\n", command);
        if ( starts_with(command, "USER") ) {
			if ( username != NULL ) {
                memset(buffer, 0, BUF_SIZE);
                strcpy(buffer, "-ERR Already provided username");
	            send(socket, buffer, strlen(buffer), 0);
				continue;
			}
			username = malloc(sizeof(char) * MAX_SIZE);
			strcpy(username, command+5);
			printf("Extracted username : '%s'", username);
			if ( check_username(username) != 0 ) {
                memset(buffer, 0, BUF_SIZE);
                strcpy(buffer, "-ERR Invalid username");
	            send(socket, buffer, strlen(buffer), 0);
				free(username);
				username = NULL;
				continue;
			}
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "+OK Username accepted");
            send(socket, buffer, strlen(buffer), 0);
        } else if ( starts_with(command, "PASS") ) {
			if ( password != NULL ) {
                memset(buffer, 0, BUF_SIZE);
                strcpy(buffer, "-ERR Already provided password");
                send(socket, buffer, strlen(buffer), 0);
				continue;
			}
			password = malloc(sizeof(char) * MAX_SIZE);
			strcpy(password, command+5);
			printf("Extracted password : '%s'", password);
			if ( check_password(username, password) != 0 ) {
                memset(buffer, 0, BUF_SIZE);
                strcpy(buffer, "-ERR Invalid password");
                send(socket, buffer, strlen(buffer), 0);
				free(password);
				password = NULL;
				continue;
			}
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "+OK Auth successful");
            send(socket, buffer, strlen(buffer), 0);
			state = 1;
        } else if ( starts_with(command, "QUIT") ) {

        } else if ( starts_with(command, "STAT") ) {

        } else if ( starts_with(command, "LIST") ) {
            handle_cmd_list(socket, username);
        } else if ( starts_with(command, "RETR") ) {

        } else if ( starts_with(command, "DELE") ) {

        } else if ( starts_with(command, "NOOP") ) {

        } else if ( starts_with(command, "RSET") ) {

        } else {
            // invalid commadn?
        }


    }
}

int main (int argc, char *argv[])
{
    if ( argc < 2 ) {
        printf("./popserver PORT\n");
        return 1;
    }

	load_usersfile();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    handle_cmd_list(0, "anandu");

    printf("Pop server\n");
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
