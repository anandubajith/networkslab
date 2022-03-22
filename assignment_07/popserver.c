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

typedef struct _mail {
    char *from;
    char *to;
    char *subject;
    char *received_time;
    char *body;
    char *body_raw;
    struct _mail* next;
    struct _mail* prev;
} Mail;

void print_mails(Mail *mail) {
    while ( mail != NULL ) {
        printf("---\n");
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
    
    if ( *head == NULL || mail == NULL)
        return;

    if ( *head == mail) {
        *head = mail->next;
    }

    if ( mail->next != NULL) {
        mail->next->prev = mail->prev;
    }

    if ( mail->prev != NULL ) {
        mail->prev->next = mail->next;
    }

    free(mail->to);
    free(mail->from);
    free(mail->subject);
    free(mail->received_time);
    free(mail->body);
    free(mail->body_raw);
    free(mail);
}

void update_emails(char*username, Mail**head) {
    char *path = get_mailbox_path(username);
    FILE *fp = fopen(path, "w");
    Mail *iter = *head;
    while( iter != NULL) {
        fprintf(fp, "from: %s\n", iter->from);
        fprintf(fp, "to: %s\n", iter->to);
        fprintf(fp, "received: %s\n", iter->received_time );
        fprintf(fp, "subject: %s\n", iter->subject);
        fprintf(fp, "%s", iter->body_raw);
        fprintf(fp, ".\n");
        iter = iter->next;
    }
    free(path);
    fclose(fp);
}

void list_messages(char *username) {
    char *path = get_mailbox_path(username);
    FILE *fp = fopen(path, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

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
    print_mails(mailHead);

}

int starts_with(char *string, char *marker) {
    assert( string != NULL );
    assert( marker != NULL );
    return strncmp(string, marker, strlen(marker)) == 0;
}

void handle_client(int socket) {
    char *command = malloc(sizeof(char) * BUF_SIZE);
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

        } else if ( starts_with(command, "PASS") ) {

        } else if ( starts_with(command, "QUIT") ) {

        } else if ( starts_with(command, "STAT") ) {

        } else if ( starts_with(command, "LIST") ) {

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

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

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
