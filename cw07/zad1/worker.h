#ifndef WORKER_H
#define WORKER_H

#include "common.h"

void init();
void set_sembuf(struct sembuf *sem_buf, int sem_num, int sem_op);

#endif //WORKER_H