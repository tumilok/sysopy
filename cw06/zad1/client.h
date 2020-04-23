#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/msg.h>

#include "common.h"

int client_q = -1;
int server_q = -1;
int chatting_q = -1;
int chatting_id = -1;
int client_id = -1;

void sigint_handler(int signal);
void error(char *msg);
char *msg_to_string(int msg);
void delete_queue();
void expected_type(const char *exp_str, long exp, long got);
void send_msg(int queue, char *msg, int mtype);
msgbuf *receive_msg(int queue);
void receive_next_msg();
void stop();
void disconnect();
void list();
void connect_to(char *msg);
void connect(char *id);
void init(key_t client_key);
void run_client();

#endif /* CLIENT_H */