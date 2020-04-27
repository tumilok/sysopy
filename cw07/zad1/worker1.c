#include "common.h"

void add()
{
	int order_value = 1 + rand() % 100;

	struct sembuf sops[3];

	set_modify(&sops[0]);
	set_free(&sops[1]);
	set_using(&sops[2]);

	if (semop(sem_id, sops, 3) == -1)
	{
		error("Could not execute operations on semaphores.");
	}

	orders *orders = shmat(orders_id, NULL, 0);

	orders -> vals[orders -> first_free] = order_value;
	orders -> first_free = (orders ->first_free + 1) % MAX_ORDERS;
	orders -> num_to_prep++;

	printf("(%d %ld) Dostalem liczbe: %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
			getpid(), time(NULL), order_value, orders -> num_to_prep, orders -> num_to_send);

	struct sembuf finalize[3];

	int semsop_idx = 0;

	if (orders -> first_free == orders -> first_to_send ||
		 orders -> first_free == orders -> first_to_prep)
	{
		finalize[semsop_idx].sem_num = ARE_FREE;		//there are no more free places
		finalize[semsop_idx].sem_op = 1;
		finalize[semsop_idx].sem_flg = 0;
		semsop_idx++;
	}

	if (semctl(sem_id, 1, GETVAL, NULL) == 1)	//if there were no more to prepare before
	{
		finalize[semsop_idx].sem_num = ARE_TO_PREP;
		finalize[semsop_idx].sem_op = -1;
		finalize[semsop_idx].sem_flg = 0;
		semsop_idx++;
	}

	finalize[semsop_idx].sem_num = IN_USE;
	finalize[semsop_idx].sem_op = -1;
	finalize[semsop_idx].sem_flg = 0;
	semsop_idx++;

	if (semop(sem_id, finalize, semsop_idx) == -1)
	{
		error("Could not execute operations on semaphores.");
	}
	shmdt(orders);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sigint_handler);

	srand(time(NULL));

	sem_id = get_sem_id();
	orders_id = get_ord_id();
	
	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		add();
	}
}