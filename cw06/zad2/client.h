#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <time.h>

#include "common.h"

char chatting_name[NAME_LEN];
char client_name[NAME_LEN];
mqd_t client_q = -1;
mqd_t server_q = -1;
mqd_t chatting_q = -1;
int client_id = -1;
int received = 0;

void sigint_handler(int signal);
void error(char *msg);
char *msg_to_string(int msg);
void apply_nonblock();
void erase_nonblock();
void set_queue_name();
void expected_type(const char *exp_str, long exp, long got);
void send_msg(mqd_t queue, char *msg, int priority);
void receive_next_msg();
void stop_handler();
void stop();
void disconnect_handler();
void disconnect();
void list();
void connect_handler(char* msg);
void connect(int client2_id);
void init();
void run_client();

#endif /* CLIENT_H */