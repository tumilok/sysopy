#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/times.h>

#define MAX_SHAVING_TIME 5
#define MAX_CLIENT_ABSENT_TIME 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_t current_client;
pthread_t *chairs;

int chairs_number;
int clients_number;
int free_chairs;

int client_next_chair = 0;
int barber_next_chair = 0;
int clients_served = 0;
int is_sleeping = 0;

void error(char *msg)
{
	printf("Error, %s\n", msg);
	exit(EXIT_FAILURE);
}

int get_rand_num(int max_waiting)
{
	return rand() % max_waiting + 1;
}

void *barber()
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        if (free_chairs == chairs_number)
        {
			is_sleeping = 1;
            printf("Golibroda: ide spac\n");
            pthread_cond_wait(&cond, &mutex);
            is_sleeping = 0;
        }
        else
        {
            current_client = chairs[barber_next_chair];
            barber_next_chair = (barber_next_chair + 1) % chairs_number;
			free_chairs++;
        }
        printf("Golibroda: czeka %d klientow, gole klienta %ld\n", chairs_number - free_chairs, current_client);
		clients_served++;
        pthread_mutex_unlock(&mutex);

        sleep(get_rand_num(MAX_SHAVING_TIME));

		if (clients_served == clients_number)
        {
            pthread_exit((void *)0);
        }
    }
}

void *client()
{
    pthread_t client_id = pthread_self();
    while (1)
    {
        pthread_mutex_lock(&mutex);
        if (is_sleeping)
        {
            current_client = client_id;
            printf("Klient: budze golibrode; %ld\n", client_id);
            pthread_cond_broadcast(&cond);
            break;
        }
        else if (free_chairs > 0)
        {
            chairs[client_next_chair] = client_id;
            client_next_chair = (client_next_chair + 1) % chairs_number;
            free_chairs--;
            printf("Klient: poczekalnia, wolne miejsca %d; %ld\n", free_chairs, client_id);
            break;
        }
        else
        {
            printf("Klient: zajete; %ld\n", client_id);
            pthread_mutex_unlock(&mutex);
            sleep(get_rand_num(MAX_CLIENT_ABSENT_TIME));
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit((void *)0);
}

int main(int args, char *argv[])
{
	if (args < 3)
	{
		error("Wrong number of arguments: Expected: [chairs_number, clients_number]");
	}

    chairs_number = atoi(argv[1]);
    clients_number = atoi(argv[2]);

	if (!clients_number)
	{
		error("Invalid clients number");
	}

	srand(time(NULL));

	chairs = calloc(chairs_number, sizeof(pthread_t));
    free_chairs = chairs_number;

	pthread_t *clients = calloc(clients_number + 1, sizeof(pthread_t));

    pthread_create(&clients[clients_number], NULL, barber, NULL);
    for (int i = 0; i < clients_number; i++)
    {
		sleep(get_rand_num(MAX_CLIENT_ABSENT_TIME));
        pthread_create(&clients[i], NULL, client, NULL);
    }

    for (int i = 0; i < clients_number + 1; i++)
    {
        pthread_join(clients[i], NULL);
    }

    return 0;
}