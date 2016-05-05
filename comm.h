#ifndef SIK_CHAT_COMM_H
#define SIK_CHAT_COMM_H

#define BUF_SIZE 1000

typedef struct message {
    uint16_t len;
    char data[BUF_SIZE];
} message;

#endif //SIK_CHAT_COMM_H
