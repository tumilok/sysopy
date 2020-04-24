#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include "common.h"

char chatting_name[NAME_LEN];
char client_name[NAME_LEN];
mqd_t client_q = -1;
mqd_t server_q = -1;
mqd_t chatting_q = -1;
int client_id = -1;
int received = 0;

#endif /* CLIENT_H */