#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>

#define MAX_MSG_SIZE 100
#define MAX_CLIENTS 10
#define SERVER_ID 154

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define CHAT 6

const char *stop_str = "STOP";
const char *disconnect_str = "DISCONNECT";
const char *list_str = "LIST";
const char *connect_str = "CONNECT";
const char *init_str = "INIT";
const char *chat_str = "CHAt";

struct msgbuf
{
    long type;
    char text[MAX_MSG_SIZE];
    int sender_id;
    int receiver_id;
};

typedef struct msgbuf msgbuf;

const size_t msgbuf_size = sizeof(msgbuf) - sizeof(long);

#endif /* COMMON_H */