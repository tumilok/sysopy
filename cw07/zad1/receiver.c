#include "common.h"

int main(int argc, char *argv[])
{
	signal(SIGINT, sigint_handler);

	srand(time(NULL));

	sem_id = get_sem_id();
	orders_id = get_ord_id();
	
	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		
		int order_value = 1 + rand() % 100;

		struct sembuf sem_buf[3];

		set_sembuf(&sem_buf[0], BUSY, 0);	// wait until nobody modifies orders
		set_sembuf(&sem_buf[1], FREE, 0);	// wait until there are free places
		set_sembuf(&sem_buf[2], BUSY, 1);	// mark orders as currently being used

		if (semop(sem_id, sem_buf, 3) == -1)
		{
			error("Could not execute operations on semaphores.");
		}

		orders *orders = shmat(orders_id, NULL, 0);

		orders -> vals[orders -> first_free] = order_value;
		orders -> first_free = (orders ->first_free + 1) % MAX_ORDERS;
		orders -> num_to_pack++;

		printf("(%d %ld) Dostalem liczbe: %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
				getpid(), time(NULL), order_value, orders -> num_to_pack, orders -> num_to_send);

		struct sembuf finalize[3];

		int index = 0;

		if (MAX_ORDERS - orders -> num_to_pack - orders -> num_to_send == 0)
		{
			set_sembuf(&finalize[index], FREE, 1);	// there are no more free places
			index++;
		}

		if (semctl(sem_id, 1, GETVAL, NULL) == 1)
		{
			set_sembuf(&finalize[index], PACK, -1);	//if there were no more to prepare before
			index++;
		}

		set_sembuf(&finalize[index], BUSY, -1);
		index++;

		if (semop(sem_id, finalize, index) == -1)
		{
			error("Could not execute operations on semaphores.");
		}
		shmdt(orders);
	}
}