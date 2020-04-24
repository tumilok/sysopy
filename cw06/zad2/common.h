#ifndef COMMON_H
#define COMMON_H

#define MAX_MSG_SIZE 100
#define MAX_CLIENTS_NUMBER 10
#define NAME_LEN 8
#define MAX_MSG 20

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define CHAT 6

const char S_QUEUE_NAME[] = "/serverqueue";

const char *refresh_str = "";
const char *stop_str = "STOP";
const char *disconnect_str = "DISCONNECT";
const char *list_str = "LIST";
const char *connect_str = "CONNECT";
const char *init_str = "INIT";

#endif /* COMMON_H */