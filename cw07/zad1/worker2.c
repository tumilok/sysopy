#include "common.h"

void prepare()
{
	struct sembuf sops[3];

	set_modify(&sops[0]);
	set_prep(&sops[1]);
	set_using(&sops[2]);

	if (semop(sem_id, sops, 3) == -1)
	{
		error("Could not execute operations on semaphores.");
	}

	orders* orders = shmat(orders_id, NULL, 0);

	orders -> vals[orders->first_to_prep] *= 2;
	int n = orders -> vals[orders -> first_to_prep];

	orders -> num_to_prep--;
	orders -> num_to_send++;
	orders -> first_to_prep = (orders->first_to_prep + 1) % MAX_ORDERS;

	printf("(%d %ld) Przygotowalem liczbe wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
		   getpid(), time(NULL), n, orders->num_to_prep, orders->num_to_send);

	struct sembuf finalize[3];

	int semsop_idx = 0;

	if (orders -> num_to_prep == 0)
	{
		finalize[semsop_idx].sem_num = ARE_TO_PREP;		//there are no more to prepare
		finalize[semsop_idx].sem_op = 1;
		finalize[semsop_idx].sem_flg = 0;
		semsop_idx++;
	}

	if (semctl(sem_id, 2, GETVAL, NULL) == 1)		//if there were no more to send before
	{
		finalize[semsop_idx].sem_num = ARE_TO_SEND;
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

	while(1)
	{
		usleep((rand() % 5 + 1) * 100000);
		prepare();
	}
}