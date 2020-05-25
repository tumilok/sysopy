#include "common.h"

int is_local;
int port;
char *server;
char *name;
int server_fd;
char symbol;
int move;
pthread_mutex_t move_mutex;

void disconnect_from_server()
{
	printf("Disconnecting from server...\n");
	send_message(server_fd, DISCONNECT, NULL, NULL);
	if (shutdown(server_fd, SHUT_RDWR) < 0)
	{
		error_exit("Could not shutdown.");
	}
	if (close(server_fd) < 0)
	{
		error_exit("Could not close server descriptor.");
	}
	exit(EXIT_SUCCESS);
}

void sigint_handler(int signal)
{
	printf("Closing client...\n");
	disconnect_from_server();
}

void local_connect_to_server()
{
	struct sockaddr_un addr;

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, server);

	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		error_exit("Socket to server failed.");
	}

	if (connect(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
	{
		error_exit("Connect to server failed.");
	}
}

void inet_connect_to_server()
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(server);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		error_exit("Socket to server failed.");
	}

	if (connect(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
	{
		error_exit("Connect to server failed.");
	}
}

void print_gameboard(game *game)
{
	for (int i = 0; i < 9; i++)
	{
		printf("%c", game -> board[i]);
		if (i % 3 == 2)
		{
			printf("\n");
		}
	}
	printf("\n");
}

void concurrent_move(void* arg)
{
	message* msg = (message*) arg;
	printf("Enter your move: ");

	int move_char = getchar();
	move = move_char - '0';

	while (move < 0 || move > 8 || msg->game.board[move] != '-')
	{
		move_char = getchar();
		move = move_char - '0';
	}
	pthread_exit(0);
}

void make_move(message *msg)
{
	move = -1;

	pthread_t move_thread;
	pthread_create(&move_thread, NULL, (void*) concurrent_move, msg);

	for( ; ; )
	{
		if (move < 0 || move > 8 || msg->game.board[move] != '-')
		{
			message rec_msg = receive_message_nonblock(server_fd);
			switch (rec_msg.message_type)
			{
				case PING:
					printf("Received PING from server. Pinging back...\n");
					send_message(server_fd, PING, NULL, NULL);
					break;
				case DISCONNECT:
					printf("Received DISCONNECT from server.\n");
					sigint_handler(SIGINT);
					exit(EXIT_SUCCESS);
				case EMPTY:
					break;
				default:
					printf("Wrong message received\n");
					break;
			}
		}
		else
		{
			break;
		}
	}

	pthread_join(move_thread, NULL);
	printf("Your move was: %d\n", move);

	msg->game.board[move] = symbol;
	print_gameboard(&msg->game);

	send_message(server_fd, MOVE, &msg->game, NULL);
}

void client_routine()
{
	for( ; ; )
	{
		message msg = receive_message(server_fd);
		switch(msg.message_type)
		{
			case WAIT:
				printf("Waiting for an opponent.\n");
				break;
			case GAME_FOUND:
				symbol = msg.game.winner;
				printf("Game started. Your symbol: %c\n", symbol);
				print_gameboard(&msg.game);
				if (symbol == 'O')
				{
					make_move(&msg);
				}
				else
				{
					printf("Waiting for enemy move\n");
				}
				break;
			case MOVE:
				printf("MOVE!\n");
				print_gameboard(&msg.game);
				make_move(&msg);
				break;
			case GAME_FINISHED:
				print_gameboard(&msg.game);
				if (msg.game.winner == symbol)
				{
					printf("YOU WON!\n");
				}
				else if(msg.game.winner == 'D')
				{
					printf("IT'S A DRAW!\n");
				}
				else
				{
					printf("YOU LOST. REALLY?\n");
				}
				disconnect_from_server();
				exit(EXIT_SUCCESS);
				break;
			case PING:
				printf("Received PING from server. Pinging back...\n");
				send_message(server_fd, PING, NULL, NULL);
				break;
			case DISCONNECT:
				printf("Received DISCONNECT from server.\n");
				sigint_handler(SIGINT);
				exit(EXIT_SUCCESS);
			default: break;
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		error_exit("Wrong number of arguments. Expected: name, local/inet, server_address");
	}

	name = argv[1];

	if (!strcmp(argv[2], "local"))
	{
		is_local = 1;
	}
	else if (!strcmp(argv[2], "inet"))
	{
		is_local = 0;
	}
	else
	{
		error_exit("Invalid arguments. Expected: name, local/inet, server_address");
	}

	srand(time(NULL));

	if (is_local)
	{
		server = argv[3];
	}
	else
	{
		if (argc < 5)
		{
			error_exit("Invalid arguments. Expected: name INET IP_adress port");
		}
		server = argv[3];
		port = atoi(argv[4]);
	}

	signal(SIGINT, sigint_handler);

	if (is_local)
	{
		local_connect_to_server();
	}
	else
	{
		inet_connect_to_server();
	}

	printf("Input a number from 0 to 8 to make a move\n");
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			printf("%d ", i + j);
		}
		printf("\n");
	}
	printf("\n");

	send_message(server_fd, CONNECT, NULL, name);

	message msg = receive_message(server_fd);

	if (msg.message_type == CONNECT)
	{
		printf("Connected to server\n");
		client_routine();
	}
	if (msg.message_type == CONNECT_FAILED)
	{
		printf("connect failed. %s\n", msg.name);
		if (shutdown(server_fd, SHUT_RDWR) < 0)
		{
			error_exit("could not shutdown.");
		}
		if (close(server_fd) < 0)
		{
			error_exit("could not close server descriptor.");
		}
		exit(EXIT_FAILURE);
	}

	printf("Something went wrong\n");
	disconnect_from_server();
}