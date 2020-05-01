#include "worker.h"

int main(int argc, char *argv[])
{
	init();

	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		
		sem_wait(free_sem);
		sem_wait(busy_sem);

		orders* orders =  get_orders();

		sem_post(busy_sem);

		int rand_num = 1 + rand() % 100;

		orders -> storage[orders -> first_free] = rand_num;
		orders -> first_free = (orders -> first_free + 1) % MAX_ORDERS_NUMBER;
		orders -> num_to_pack++;

		printf("(%d %ld) Dostalem liczbe: %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
			getpid(), time(NULL), rand_num, orders -> num_to_pack, orders -> num_to_send);

		// setting free sem if there are still free places
		if (!(orders -> first_free == orders -> first_to_send ||
			orders -> first_free == orders -> first_to_pack))
		{
			sem_post(free_sem);
		}

		int val;
		sem_getvalue(pack_sem, &val);
		// setting pack sem if it wasn't already set
		if(!val)
		{				
			sem_post(pack_sem);
		}
		unmount_orders(orders);
		sem_post(busy_sem);
	}
}