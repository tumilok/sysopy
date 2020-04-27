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
#define WORKER1_NUM 5
#define WORKER2_NUM 2
#define WORKER3_NUM 3

typedef struct 
{
	int first_free;
	int first_to_prep;
	int first_to_send;
	int num_to_prep;
	int num_to_send;
	int vals[MAX_ORDERS];
} orders;

union semun 
{
	int val;	
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

#define	IN_USE 0
#define	ARE_TO_PREP 1
#define	ARE_TO_SEND 2
#define	ARE_FREE 3

int sem_id;
int orders_id;

void error(char *msg);
void sigint_handler(int signal);
key_t get_sem_key();
key_t get_ord_key();
int get_sem_id();
int get_ord_id();
void set_modify(struct sembuf *sem_buf);
void set_using(struct sembuf *sem_buf);
void set_free(struct sembuf *sem_buf);
void set_prep(struct sembuf *sem_buf);
void set_orders(struct sembuf *sem_buf);

#endif //COMMON_H