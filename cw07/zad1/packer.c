#include "worker.h"

int main(int argc, char *argv[])
{
	init();

	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		
		struct sembuf sem_buf[3];

		set_sembuf(&sem_buf[0], BUSY, 0);	// wait until nobody modifies orders
		set_sembuf(&sem_buf[1], PACK, 0);	// wait until there are orders to prepare
		set_sembuf(&sem_buf[2], BUSY, 1);	// mark orders as currently being used

		if (semop(sem_id, sem_buf, 3) == -1)
		{
			error("Could not execute operations on semaphores.");
		}

		orders* orders = shmat(orders_id, NULL, 0);

		int n = orders -> storage[orders -> first_to_pack] *= 2;
		orders -> num_to_pack--;
		orders -> num_to_send++;
		orders -> first_to_pack = (orders -> first_to_pack + 1) % MAX_ORDERS;

		printf("(%d %ld) Przygotowalem liczbe wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
			getpid(), time(NULL), n, orders -> num_to_pack, orders -> num_to_send);

		struct sembuf new_flags[3];

		int index = 0;

		// setting pack sem if there is still smth to pack
		if (orders -> num_to_pack == 0)
		{
			set_sembuf(&new_flags[index], PACK, 1);
			index++;
		}

		// setting send sem if it wasn't already set
		if (semctl(sem_id, 2, GETVAL, NULL) == 1)
		{
			set_sembuf(&new_flags[index], SEND, -1);
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