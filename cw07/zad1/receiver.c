#include "worker.h"

int main(int argc, char *argv[])
{
	init();
	
	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);

		struct sembuf sem_buf[3];

		set_sembuf(&sem_buf[0], BUSY, 0);	// wait until nobody modifies orders
		set_sembuf(&sem_buf[1], FREE, 0);	// wait until there are free places
		set_sembuf(&sem_buf[2], BUSY, 1);	// mark orders as currently being used

		if (semop(sem_id, sem_buf, 3) == -1)
		{
			error("Could not execute operations on semaphores.");
		}

		orders *orders = shmat(orders_id, NULL, 0);

		int rand_num = 1 + rand() % 100;

		orders -> storage[orders -> first_free] = rand_num;
		orders -> first_free = (orders ->first_free + 1) % MAX_ORDERS;
		orders -> num_to_pack++;

		printf("(%d %ld) Dostalem liczbe: %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
				getpid(), time(NULL), rand_num, orders -> num_to_pack, orders -> num_to_send);

		struct sembuf new_flags[3];

		int index = 0;

		// setting free sem if there are still free places
		if (!(orders -> first_free == orders -> first_to_send ||
			orders -> first_free == orders -> first_to_pack))
		{
			set_sembuf(&new_flags[index], FREE, 1);
			index++;
		}

		// setting pack sem if it wasn't already set
		if (semctl(sem_id, 1, GETVAL, NULL) == 1)
		{
			set_sembuf(&new_flags[index], PACK, -1);
			index++;
		}

		set_sembuf(&new_flags[index], BUSY, -1);
		index++;

		if (semop(sem_id, new_flags, index) == -1)
		{
			error("Could not execute operations on semaphores.");
		}
		shmdt(orders);
	}
}