#include "common.h"

void send()
{
	struct sembuf sops[3];

	set_modify(&sops[0]);
	set_orders(&sops[1]);
	set_using(&sops[2]);

	if(semop(sem_id, sops, 3) == -1)
	{
		error("Could not execute operations on semaphores.");
	}

	orders* orders = shmat(orders_id, NULL, 0);

	orders -> vals[orders->first_to_send] *= 3;
	int n = orders->vals[orders->first_to_send];

	orders -> num_to_send--;
	orders -> first_to_send = (orders -> first_to_send + 1) % MAX_ORDERS;

	printf("(%d %ld) Wyslalem zamowienie wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
		   getpid(), time(NULL), n, orders -> num_to_prep, orders -> num_to_send);

	struct sembuf finalize[3];

	int semsop_idx = 0;

	if (orders -> num_to_send == 0)
	{
		finalize[semsop_idx].sem_num = ARE_TO_SEND;		//there are no more to send
		finalize[semsop_idx].sem_op = 1;
		finalize[semsop_idx].sem_flg = 0;
		semsop_idx++;
	}

	if (semctl(sem_id, 3, GETVAL, NULL) == 1)		//if there were no free places before
	{
		finalize[semsop_idx].sem_num = ARE_FREE;
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
		send();
	}
}