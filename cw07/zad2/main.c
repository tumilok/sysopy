#include "common.h"

pid_t worker_pids[WORKERS_NUM];

void terminate()
{
	for (int i = 0; i < WORKERS_NUM; i++)
	{
		kill(worker_pids[i], SIGINT);
	}

	sem_unlink(BUSY);
	sem_unlink(PACK);
	sem_unlink(SEND);
	sem_unlink(FREE);

	if (shm_unlink(ORDER_NAME) == -1)
	{
		error("Could not delete shared memory.");
	}
}

sem_t *create_sem(char *name, int value)
{
	sem_t* sem = sem_open(name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, value);
	if (sem == SEM_FAILED)
	{
		error("Couldn't create semaphore");
	}
	return sem;
}

void create_sems()
{
	sem_close(create_sem(BUSY, 1));
	sem_close(create_sem(FREE, 1));
	sem_close(create_sem(PACK, 0));
	sem_close(create_sem(SEND, 0));
}

void create_orders()
{
	int orders_fd;
	if ((orders_fd = shm_open(ORDER_NAME, O_RDWR | O_CREAT, 0666)) == -1)
	{
		error("Couldn't create shared memory");
	}

	if (ftruncate(orders_fd, sizeof(orders)) == -1)
	{
		error("Couldn't define size of shared memory");
	}

	orders* orders = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE, MAP_SHARED, orders_fd, 0);

	orders -> num_to_pack = 0;
	orders -> num_to_send = 0;
	orders -> first_free = 0;
	orders -> first_to_pack = 0;
	orders -> first_to_send = 0;

	for (int i = 0; i < MAX_ORDERS_NUMBER; i++)
	{
		orders -> storage[i] = -1;
	}

	if (munmap(orders, sizeof(orders)) == -1)
	{
		error("Couldn't unmount shared memory");
	}
}

int main(int argc, char *argv[])
{
	atexit(terminate);
	signal(SIGINT, sigint_handler);

	create_sems();
	create_orders();

	for (int i = 0; i < WORKERS_NUM; i++)
	{
		if ((worker_pids[i] = fork()) == 0) {
			if (i < RECEIVER_NUM)
			{
				execlp("./receiver", "receiver", NULL);
			}
			else if (i < RECEIVER_NUM + PACKER_NUM)
			{
				execlp("./packer", "packer", NULL);
			}
			else
			{
				execlp("./sender", "sender", NULL);
			}
		}
	}
	
	for (int i = 0; i < WORKERS_NUM; i++)
	{
		wait(NULL);
	}
}