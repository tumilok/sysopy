#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>

#include "common.h"

typedef struct
{
	char name[NAME_LEN];
	mqd_t queue;
	int chatting_id;
} client;

client clients[MAX_CLIENTS_NUMBER];
mqd_t server_q = -1;

void sigint_handler(int signal);
void error(char *msg);
char *msg_to_string(int msg);
int get_index();
void send_msg(mqd_t queue, char *msg, int priority);
void close_queue(int client_id);
void stop_client(int client_id);
void disconnect(int client1_id);
void list(int client_id);
void send_connect_msg(int client1_id, int client2_id);
void connect(char* msg);
void init(char *msg, int priority);
void terminate_server();
void run_server();

#endif /* SERVER_H */