#define PACKET_SIZE 500
#define TIMEOUT 123

typedef struct _message {
    int seq_no;
    int ack_no;
    int size;
    char data[PACKET_SIZE];
} Message;




/*
 * Structure?
 *
 *
 */
