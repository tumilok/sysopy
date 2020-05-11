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

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <linux/limits.h>

#define MAX_ORDERS 10

#define SEM_ID 's'
#define ORD_ID 'o'

#define SEM_NUMBER 4

#define RECEIVER_NUM 100
#define PACKER_NUM 1
#define SENDER_NUM 200
#define WORKERS_NUM                            \
  (RECEIVER_NUM + PACKER_NUM + SENDER_NUM)

#define	BUSY 0
#define	PACK 1
#define	SEND 2
#define	FREE 3

typedef struct 
{
	int first_free;
	int first_to_pack;
	int first_to_send;
	int num_to_pack;
	int num_to_send;
	int storage[MAX_ORDERS];
} orders;

union semun 
{
	int val;	
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int sem_id;
int orders_id;

void error(char *msg);
void sigint_handler(int signal);
key_t get_sem_key();
key_t get_ord_key();
int get_sem_id();
int get_ord_id();

#endif //COMMON_H