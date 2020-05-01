#include "common.h"

void error(char *msg)
{
	printf("%s Error: %s\n", msg, strerror(errno));
	exit(EXIT_FAILURE);
}

void sigint_handler(int signal)
{
	exit(EXIT_SUCCESS);
}

key_t get_sem_key()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof cwd);
    return ftok(cwd, SEM_ID);
}

key_t get_ord_key()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof cwd);
    return ftok(cwd, ORD_ID);
}

int get_sem_id()
{
	int sem_id = semget(get_sem_key(), 0, 0);
	if (sem_id == -1)
	{
		error("Could not access semaphores.");
	}
    return sem_id;
}

int get_ord_id()
{
	int orders_id = shmget(get_ord_key(), 0, 0);
	if (orders_id == -1)
	{
		error("Could not access shared memory.");
	}
    return orders_id;
}