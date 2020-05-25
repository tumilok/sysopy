#include "common.h"

#define PING_INTERVAL 10
#define PING_WAIT 5

struct sockaddr local_sockaddr;
struct sockaddr_in inet_sockaddr;

client clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex;

pthread_t net_thread;
pthread_t ping_thread;

int port;
char *socket_path;
int local_socket;
int inet_socket;
int waiting_idx = -1;
int first_free = 0;

void sigint_handler(int signal)
{
	exit(EXIT_SUCCESS);
}

void close_server()
{
	if (pthread_cancel(net_thread) == -1)
	{
		error_exit("Could not cancel net tread");
	}
	if (pthread_cancel(ping_thread) == -1)
	{
		error_exit("Could not cancel ping thread");
	}
	close(local_socket);
	unlink(socket_path);
	close(inet_socket);
}

void terminate()
{
	close_server();
}

int is_client(int i)
{
	return i >= 0 && i < MAX_CLIENTS && clients[i].fd != -1;
}

void delete_game(game* game)
{
	free(game);
}

int get_free_idx()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!is_client(i))
		{
			return i;
		}
	}
	return -1;
}

int get_client_index(char *name)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (is_client(i) && !strcmp(name, clients[i].name))
		{
			return i;
		}
	}
	return -1;
}

int is_name_available(char *name)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (is_client(i) && strcmp(name, clients[i].name) == 0)
		{
			return 0;
		}
	}
	return 1;
}

void start_local()
{
	if((local_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
	{
		error_exit("Local socket initialization failed.");
	}

	local_sockaddr.sa_family = AF_UNIX;
	strcpy(local_sockaddr.sa_data, socket_path);

	int local_bind;
	if((local_bind = bind(local_socket, &local_sockaddr, sizeof(local_sockaddr))) == -1)
	{
		error_exit("Local bind failed.");
	}
}

void start_inet()
{
	inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (inet_socket == -1)
	{
		error_exit("Inet socket initialization failed");
	}

	inet_sockaddr.sin_family = AF_INET;
	inet_sockaddr.sin_port = htons(port);
	inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int inet_bind;
	if((inet_bind = bind(inet_socket, (struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr))) == -1)
	{
		error_exit("Inet bind failed.");
	}
}

void empty_client(int i)
{
	if (clients[i].name != NULL)
	{
		free(clients[i].name);
	}
	if (clients[i].addr != NULL)
	{
		free(clients[i].addr);
	}
	clients[i].name = NULL;
	clients[i].addr = NULL;
	clients[i].fd = -1;
	clients[i].game = NULL;
	clients[i].active = 0;
	clients[i].symbol = '-';
	clients[i].opponent_idx = -1;
	if (waiting_idx == i)
	{
		waiting_idx = -1;
	}
}

void process_move(game *game)
{
	static int wins[8][3] = { {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
							  {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
							  {0, 4, 8}, {2, 4, 6} };

	for (int i = 0; i < 8; i++)
	{

		char winning_char = game->board[wins[i][0]];
		if (game -> board[wins[i][1]] == winning_char
			&& game -> board[wins[i][2]] == winning_char)
			{
			game -> winner = winning_char;
			return;
		}
	}

	int any_empty = 0;
	for (int i = 0; i < 9; i++)
	{
		if (game->board[i] == '-')
		{
			any_empty = 1;
			break;
		}
	}

	if (any_empty == 0)
	{
		game->winner = 'D';
	}

	if (game -> turn == 'X')
	{
		game -> turn = 'O';
	}
	else if (game->turn == 'O')
	{
		game->turn = 'X';
	}
}

void start_game(int id1, int id2)
{
	clients[id1].opponent_idx = id2;
	clients[id2].opponent_idx = id1;

	int beg = rand() % 2;
	clients[id1].symbol = (beg == 0) ? 'O' : 'X';
	clients[id2].symbol = (beg == 0) ? 'X' : 'O';

	game *game = malloc(sizeof(game));
	empty_game_board(game);

	clients[id1].game = clients[id2].game = game;

	game -> winner = clients[id1].symbol;
	send_message_to(clients[id1].fd, clients[id1].addr, GAME_FOUND, game, clients[id1].name);

	game -> winner = clients[id2].symbol;
	send_message_to(clients[id2].fd, clients[id2].addr, GAME_FOUND, game, clients[id2].name);

	game -> winner = '-';
}

void connect_client(int fd, struct sockaddr *addr, char *rec_name)
{
	printf("Connecting client\n");

	char *name = calloc(NAME_LENGTH, sizeof(char));
	strcpy(name, rec_name);

	if (!is_name_available(name))
	{
		send_message_to(fd, addr, CONNECT_FAILED, NULL, "Your nick is already taken");
		free(name);
		return;
	}
	if (first_free == -1)
	{
		send_message_to(fd, addr, CONNECT_FAILED, NULL, "Server is full");
		free(name);
		return;
	}

	send_message_to(fd, addr, CONNECT, NULL, "Connected");

	clients[first_free].name = name;
	clients[first_free].active = 1;
	clients[first_free].fd = fd;
	clients[first_free].addr = addr;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (clients[i].name != NULL)
		{
			printf("%d: %s\n",i, clients[i].name);
		}
	}

	if (waiting_idx != -1)
	{
		start_game(first_free, waiting_idx);
		waiting_idx = -1;
	}
	else
	{
		waiting_idx = first_free;
		send_message_to(fd, clients[first_free].addr, WAIT, NULL, name);
		printf("WAIT sent\n");
	}

	first_free=get_free_idx();
	printf("Connected\n");
}

void net_routine(void *arg)
{
	struct pollfd poll_fds[2];

	poll_fds[0].fd = local_socket;
	poll_fds[1].fd = inet_socket;
	poll_fds[0].events = POLLIN;
	poll_fds[1].events = POLLIN;

	while (1)
	{
		pthread_mutex_lock(&clients_mutex);
		for (int i = 0; i < 2; i++)
		{
			poll_fds[i].events = POLLIN;
			poll_fds[i].revents = 0;
		}
		pthread_mutex_unlock(&clients_mutex);

		printf("Polling...\n");
		if (poll(poll_fds, 2, -1) == -1)
		{
			error_exit("Poll failed.");
		}
		pthread_mutex_lock(&clients_mutex);

		for (int i = 0; i < 2; i++)
		{
			if (poll_fds[i].revents && POLLIN)
			{
				struct sockaddr* addr = malloc(sizeof(struct sockaddr));
				socklen_t len = sizeof(&addr);
				printf("DESCRIPTOR: %d\n", poll_fds[i].fd);
				message msg = receive_message_from(poll_fds[i].fd, addr, len);
				printf("Message received in server\n");
				int j;
				switch (msg.message_type)
				{
					case CONNECT:
						connect_client(poll_fds[i].fd, addr, msg.name);
						break;
					case MOVE:
						printf("Received move\n");
						j = get_client_index(msg.name);
						process_move(&msg.game);
						if (msg.game.winner == '-')
						{
							send_message_to(clients[clients[j].opponent_idx].fd, clients[clients[j].opponent_idx].addr,
							 	MOVE, &msg.game, clients[clients[j].opponent_idx].name);
						}
						else
						{
							send_message_to(clients[j].fd, clients[j].addr, GAME_FINISHED, &msg.game, clients[j].name);
							send_message_to(clients[clients[j].opponent_idx].fd, clients[clients[j].opponent_idx].addr,
								 GAME_FINISHED, &msg.game, clients[clients[j].opponent_idx].name);
							delete_game(clients[j].game);
						}
						free(addr);
						break;
					case PING:
						j = get_client_index(msg.name);
						clients[j].active = 1;
						free(addr);
						break;
					case DISCONNECT:
						j = get_client_index(msg.name);
						printf("Received disconnect from client\n");
						if (is_client(clients[j].opponent_idx))
						{
							send_message_to(clients[clients[j].opponent_idx].fd, clients[clients[j].opponent_idx].addr,
								 DISCONNECT, NULL, clients[clients[j].opponent_idx].name);
							empty_client(clients[j].opponent_idx);
						}
						empty_client(j);
						free(addr);
						break;
					default:
						free(addr);
						break;
				}
			}
		}
		pthread_mutex_unlock(&clients_mutex);
	}
}

void ping_routine(void *arg)
{
	while (1)
	{
		sleep(PING_INTERVAL);
		printf("Ping in progress...\n");

		pthread_mutex_lock(&clients_mutex);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if (is_client(i))
			{
				clients[i].active = 0;
				send_message_to(clients[i].fd, clients[i].addr, PING, NULL, clients[i].name);
			}
		}
		pthread_mutex_unlock(&clients_mutex);

		sleep(PING_WAIT);

		pthread_mutex_lock(&clients_mutex);
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (is_client(i) && clients[i].active == 0)
			{
				printf("Response from %d was not received. Disconnecting %d...\n", i, i);
				send_message_to(clients[i].fd, clients[i].addr, DISCONNECT, NULL, clients[i].name);
				if (is_client(clients[i].opponent_idx))
				{
					send_message_to(clients[clients[i].opponent_idx].fd, clients[clients[i].opponent_idx].addr,
						 DISCONNECT, NULL, clients[clients[i].opponent_idx].name);
					empty_client(clients[i].opponent_idx);
				}
				empty_client(i);
			}
		}
		pthread_mutex_unlock(&clients_mutex);
		printf("Ping ended.\n");
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		error_exit("wrong number of arguments. Expected: port, socket_path");
	}

	port = atoi(argv[1]);
	socket_path = argv[2];

	signal(SIGINT, sigint_handler);
	atexit(terminate);

	srand(time(NULL));

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		empty_client(i);
	}

	start_local();
	start_inet();
	printf("Started...\n");

	if (pthread_create(&net_thread, NULL, (void*) net_routine, NULL) == -1)
	{
		error_exit("could not create net thread.");
	}
	if (pthread_create(&ping_thread, NULL, (void*) ping_routine, NULL) == -1)
	{
		error_exit("could not create ping thread");
	}

	if (pthread_join(net_thread, NULL) < 0)
	{
		error_exit("Could not join net thread.");
	}
	if (pthread_join(ping_thread, NULL) < 0)
	{
		error_exit("Could not join ping thread.");
	}

	close_server();
	exit(EXIT_SUCCESS);
}