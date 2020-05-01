#include "worker.h"

int main(int argc, char *argv[])
{
	init();

	while (1)
	{
		usleep((rand() % 5 + 1) * 100000);
		
		sem_wait(send_sem);
		sem_wait(busy_sem);

		orders* orders =  get_orders();

		sem_post(busy_sem);

		if (orders -> num_to_send > 0)
		{
			int n = orders -> storage[orders -> first_to_send] *= 3;
			orders -> num_to_send--;
			orders -> first_to_send = (orders -> first_to_send + 1) % MAX_ORDERS_NUMBER;

			printf("(%d %ld) Wyslalem zamowienie wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d\n",
				getpid(), time(NULL), n, orders -> num_to_pack, orders -> num_to_send);

			int val;
			sem_getvalue(free_sem, &val);
			// setting free sem if it wasn't already set
			if (!val)
			{
				sem_post(free_sem);
			}
		}

		//if there are still any to send
		if (orders -> num_to_send > 0)
		{	
			sem_post(send_sem);
		}
		unmount_orders(orders);
		sem_post(busy_sem);
	}
}