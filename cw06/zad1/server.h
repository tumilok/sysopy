#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/msg.h>

#include "common.h"

typedef struct
{
    pid_t pid;
    key_t queue_key;
    int chatting_id;
} client;

client clients[MAX_CLIENTS_NUMBER];
int server_q = -1;

void sigint_handler(int signal);
void error(char *msg);
char *msg_to_string(int msg);
void send_msg(int queue, char *msg, int mtype);
void stop_client(int client_id);
void disconnect(int client1_id);
void list(int client_id);
void send_connect_msg(int client_id, int chatting_id);
void connect(int client1_id, int client2_id);
void init(msgbuf msg_buf);
void terminate_server();
void run_server();

#endif /* SERVER_H */