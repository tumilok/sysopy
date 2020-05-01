#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <linux/limits.h>

#define MAX_ORDERS_NUMBER 10

#define RECEIVER_NUM 3
#define PACKER_NUM 3
#define SENDER_NUM 3
#define WORKERS_NUM                            \
  (RECEIVER_NUM + PACKER_NUM + SENDER_NUM)

#define ORDER_NAME "order_name"

#define	BUSY "busy"
#define	FREE "free"
#define	PACK "pack"
#define	SEND "send"

typedef struct 
{
	int first_free;
	int first_to_pack;
	int first_to_send;
	int num_to_pack;
	int num_to_send;
	int storage[MAX_ORDERS_NUMBER];
} orders;

void error(char *msg);
void sigint_handler(int signal);

#endif //COMMON_H