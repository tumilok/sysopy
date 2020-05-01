#include "worker.h"

void init()
{
    signal(SIGINT, sigint_handler);

	srand(time(NULL));

	sem_id = get_sem_id();
	orders_id = get_ord_id();
}

void set_sembuf(struct sembuf *sem_buf, int sem_num, int sem_op)
{
	sem_buf -> sem_num = sem_num;
	sem_buf -> sem_op = sem_op;
	sem_buf -> sem_flg = 0;
}