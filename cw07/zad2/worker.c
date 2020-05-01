#include "worker.h"

orders *get_orders()
{
    return mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE, MAP_SHARED, orders_fd, 0);
}

void unmount_orders(orders* orders)
{
    if (munmap(orders, sizeof(orders)) == -1)
    {
        error("Couldn't unmount shared memory");
    }
}

void terminate()
{
	sem_close(busy_sem);
    sem_close(free_sem);
	sem_close(pack_sem);
	sem_close(send_sem);
}

sem_t *open_sem(char *who)
{
	sem_t *sem = sem_open(who, O_RDWR);
	if (sem == SEM_FAILED)
    {
        error("Could not create semaphore.");
    }
	return sem;
}

void open_sems()
{
    busy_sem = open_sem(BUSY);
	pack_sem = open_sem(PACK);
	send_sem = open_sem(SEND);
	free_sem = open_sem(FREE);
}

void init()
{
    atexit(terminate);
	signal(SIGINT, sigint_handler);
	
	srand(time(NULL));

	open_sems();

	if ((orders_fd = shm_open(ORDER_NAME, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
	{
		error("Couldn't open shared memory");
	} 
}