#ifndef WORKER_H
#define WORKER_H

#include "common.h"

sem_t *busy_sem;
sem_t *free_sem;
sem_t *pack_sem;
sem_t *send_sem;

int orders_fd;

orders *get_orders();
void unmount_orders(orders* orders);
void init();

#endif //WORKER_H