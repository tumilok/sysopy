#include "worker.h"

int main(int argc, char *argv[])
{
	init();

	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		
		sem_wait(pack_sem);
		sem_wait(busy_sem);

		orders* orders =  get_orders();

		sem_post(busy_sem);

		if (orders -> num_to_pack > 0)
		{
			int n = orders -> storage[orders -> first_to_pack] *= 2;
			orders -> num_to_pack--;
			orders -> num_to_send++;
			orders -> first_to_pack = (orders->first_to_pack + 1) % MAX_ORDERS_NUMBER;

			printf("(%d %ld) Przygotowalem liczbe wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
				getpid(), time(NULL), n, orders -> num_to_pack, orders -> num_to_send);

			int val;
			sem_getvalue(send_sem, &val);
			// setting send sem if it wasn't already set
			if (!val)
			{
				sem_post(send_sem);
			}
		}
		
		// setting pack sem if there is still smth to pack
		if (orders -> num_to_pack > 0)
		{
			sem_post(pack_sem);
		}
		unmount_orders(orders);
		sem_post(busy_sem);
	}
}