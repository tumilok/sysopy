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

void start_local()
{
	if ((local_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		error_exit("local socket initialization failed");
	}

	int local_bind;
	int local_listen;
	local_sockaddr.sa_family = AF_UNIX;
	strcpy(local_sockaddr.sa_data, socket_path);

	if ((local_bind = bind(local_socket, &local_sockaddr, sizeof(local_sockaddr))) == -1)
	{
		error_exit("local bind failed.");
	}

	if ((local_listen = listen(local_socket, MAX_CLIENTS)) == -1)
	{
		error_exit("local listen failed.");
	}
}

void start_inet()
{
	if ((inet_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		error_exit("inet socket initialization failed");
	}

	int inet_bind;
	int inet_listen;
	inet_sockaddr.sin_family = AF_INET;
	inet_sockaddr.sin_port = htons(port);
	inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if ((inet_bind = bind(inet_socket, (struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr))) == -1)
	{
		error_exit("inet bind failed.");
	}

	if ((inet_listen = listen(inet_socket, MAX_CLIENTS)) == -1)
	{
		error_exit("inet listen failed.");
	}
}

void disconnect_client(int i)
{
	printf("Disconnecting %s\n", clients[i].name);
	if (!is_client(i))
	{
		return;
	}
	if (shutdown(clients[i].fd, SHUT_RDWR) < 0)
	{
		error_exit("Could not disconnect client.");
	}
	if (close(clients[i].fd) < 0)
	{
		error_exit("Could not close client.");
	}
}

void empty_client(int i)
{
	if (clients[i].name != NULL)
	{
		free(clients[i].name);
	}
	clients[i].name = NULL;
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

void process_move(game* game)
{
	static int wins[8][3] = { 
		{0, 1, 2}, {3, 4, 5}, {6, 7, 8},
		{0, 3, 6}, {1, 4, 7}, {2, 5, 8},
		{0, 4, 8}, {2, 4, 6} 
	};

	for (int i = 0; i < 8; i++)
	{
		char winning_char = game->board[wins[i][0]];
		if (game->board[wins[i][1]] == winning_char
			&& game->board[wins[i][2]] == winning_char)
			{
			game->winner = winning_char;
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

	if (game->turn == 'X')
	{
		game->turn = 'O';
	}
	else if (game->turn == 'O')
	{
		game->turn = 'X';
	}
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

void start_game(int id1, int id2)
{
	clients[id1].opponent_idx = id2;
	clients[id2].opponent_idx = id1;

	int beg = rand() % 2;
	clients[id1].symbol = (beg == 0) ? 'O' : 'X';
	clients[id2].symbol = (beg == 0) ? 'X' : 'O';

	//if message type is GAME_FOUND, then winner means only whose symbol it is

	game *new_game = malloc(sizeof(game));
	empty_game_board(new_game);

	clients[id1].game = clients[id2].game = new_game;

	new_game -> winner = clients[id1].symbol;
	send_message(clients[id1].fd, GAME_FOUND, new_game, NULL);

	new_game -> winner = clients[id2].symbol;
	send_message(clients[id2].fd, GAME_FOUND, new_game, NULL);

	new_game -> winner = '-';
}

void connect_client(int fd)
{
	printf("Connecting client\n");

	int client_fd = accept(fd, NULL, NULL);
	if (client_fd < 0)
	{
		error_exit("Could not accept client.");
	}

	message msg = receive_message(client_fd);

	char *name = calloc(NAME_LENGTH, sizeof(char));

	strcpy(name, msg.name);

	if (!is_name_available(name))
	{
		send_message(client_fd, CONNECT_FAILED, NULL, "Your name is already taken");
		return;
	}
	if (first_free == -1)
	{
		send_message(client_fd, CONNECT_FAILED, NULL, "Server is full");
	}

	send_message(client_fd, CONNECT, NULL, "Connected");

	clients[first_free].name = name;
	clients[first_free].active = 1;
	clients[first_free].fd = client_fd;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (clients[i].name != NULL)
		{
			printf("%d: %s\n", i, clients[i].name);
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
		send_message(client_fd, WAIT, NULL, NULL);
		printf("WAIT sent\n");
	}

	first_free = get_free_idx();

	printf("Connected\n");
}

void net_routine(void *arg)
{
	struct pollfd poll_fds[MAX_CLIENTS + 2];

	poll_fds[MAX_CLIENTS].fd = local_socket;
	poll_fds[MAX_CLIENTS + 1].fd = inet_socket;

	while (1)
	{
		pthread_mutex_lock(&clients_mutex);
		for (int i = 0; i < MAX_CLIENTS + 2; i++)
		{
			if (i < MAX_CLIENTS)
			{
				poll_fds[i].fd = clients[i].fd;
			}
			poll_fds[i].events = POLLIN;
			poll_fds[i].revents = 0;
		}
		pthread_mutex_unlock(&clients_mutex);

		printf("Polling...\n");
		if (poll(poll_fds, MAX_CLIENTS + 2, -1) == -1)
		{
			error_exit("Poll failed.");
		}

		pthread_mutex_lock(&clients_mutex);
		for (int i = 0; i < MAX_CLIENTS + 2; i++)
		{
			if (i < MAX_CLIENTS && !is_client(i))
			{
				continue;
			}

			if (poll_fds[i].revents && POLLIN)
			{
				if (poll_fds[i].fd == local_socket || poll_fds[i].fd == inet_socket)
				{
					connect_client(poll_fds[i].fd);
				}
				else
				{
					message msg = receive_message(poll_fds[i].fd);
					switch (msg.message_type)
					{
						case MOVE:
							printf("Received move\n");
							process_move(&msg.game);
							if (msg.game.winner == '-')
							{
								send_message(clients[clients[i].opponent_idx].fd, MOVE, &msg.game, NULL);
							}
							else
							{
								send_message(poll_fds[i].fd, GAME_FINISHED, &msg.game, NULL);
								send_message(clients[clients[i].opponent_idx].fd, GAME_FINISHED, &msg.game, NULL);
								delete_game(clients[i].game);
							}
							break;
						case PING:
							clients[i].active = 1;
							break;
						case DISCONNECT:
							printf("Received disconnect from client\n");
							if (is_client(clients[i].opponent_idx))
							{
								disconnect_client(clients[i].opponent_idx);
								empty_client(clients[i].opponent_idx);
							}
							disconnect_client(i);
							empty_client(i);
							break;
						default:
							break;
					}
				}
			}
			else if(is_client(i) && poll_fds[i].revents && POLLHUP)
			{
				printf("Disconnect...\n");
				disconnect_client(i);
				empty_client(i);
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
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (is_client(i))
			{
				clients[i].active = 0;
				send_message(clients[i].fd, PING, NULL, NULL);
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
				send_message(clients[i].fd, DISCONNECT, NULL, NULL);
				if (is_client(clients[i].opponent_idx))
				{
					disconnect_client(clients[i].opponent_idx);
					empty_client(clients[i].opponent_idx);
				}
				disconnect_client(i);
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

	atexit(terminate);
	signal(SIGINT, sigint_handler);

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		empty_client(i);
	}

	start_local();
	start_inet();
	printf("Server was successfully started\n");

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
		error_exit("could not join net thread.");
	}
	if (pthread_join(ping_thread, NULL) < 0)
	{
		error_exit("could not join ping thread.");
	}

	close_server();
	exit(EXIT_SUCCESS);
}