typedef struct _message {
    int seq_no;
    int type;
    int offset;
    char data[500];
} Message;
