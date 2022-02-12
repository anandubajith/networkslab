#define PACKET_SIZE 500
#define TIMEOUT 123

typedef struct _message {
    int seq_no;
    int type;
    int ack;
    int size;
    char data[PACKET_SIZE];
} Message;




/*
 * Structure?
 *
 *
 */
