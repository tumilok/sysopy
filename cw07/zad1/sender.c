#include "worker.h"

int main(int argc, char *argv[])
{
	init();

	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		
		struct sembuf sem_buf[3];

		set_sembuf(&sem_buf[0], BUSY, 0);	// wait until nobody modifies orders
		set_sembuf(&sem_buf[1], SEND, 0);	// wait until there are orders to send
		set_sembuf(&sem_buf[2], BUSY, 1);	// mark orders as currently being used

		if (semop(sem_id, sem_buf, 3) == -1)
		{
			error("Could not execute operations on semaphores.");
		}

		orders* orders = shmat(orders_id, NULL, 0);

		int n = orders -> storage[orders->first_to_send] *= 3;
		orders -> num_to_send--;
		orders -> first_to_send = (orders -> first_to_send + 1) % MAX_ORDERS;

		printf("(%d %ld) Wyslalem zamowienie wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
			getpid(), time(NULL), n, orders -> num_to_pack, orders -> num_to_send);

		struct sembuf new_flags[3];

		int index = 0;

		//if there are still any to send
		if (orders -> num_to_send == 0)
		{
			set_sembuf(&new_flags[index], SEND, 1);
			index++;
		}

		// setting free sem if it wasn't already set
		if (semctl(sem_id, 3, GETVAL, NULL) == 1)
		{
			set_sembuf(&new_flags[index], FREE, -1);
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