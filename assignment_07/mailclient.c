#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 100

void handle_manage_mail(int server_port, char *username, char *password) {
    printf("Handle manage email\n");
}

void handle_send_mail(int server_port, char *username, char *password) {
    // create socket connection to the SMTP server
    printf("Handle send email\n");

}

int main (int argc, char *argv[]) {
    if ( argc < 3 ) {
        printf("./mailclient SMTP_PORT POP_PORT\n");
        return 1;
    }

    char *username = malloc(sizeof(char) * MAX_SIZE);
    char *password = malloc(sizeof(char) * MAX_SIZE);

    // ask for username
    printf("username: ");
    fflush(stdout);
    scanf("%s", username);
    // ask for password
    printf("password: ");
    fflush(stdout);
    scanf("%s", password);


    int input = -1;
    while(1){
        printf("\x1b[2J\x1b[H"); // clear and position cursor at top
        printf("Welcome %s\n", "asdf");
        printf("1. Manage mail\n2. Send Mail\n3. Quit\n");
        scanf("%d", &input);
        /* printf("got input %d", input); */
        if ( input == 1) {
            handle_manage_mail(atoi(argv[2]), username, password);
        } else if ( input == 2) {
            handle_send_mail(atoi(argv[1]), username, password);
        } else if ( input == 3) {
            return 0;
        }
        input = -1;
    }


    return 0;
}
