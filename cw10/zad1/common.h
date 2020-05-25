#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/sockios.h>

#define MAX_CLIENTS 13
#define MSG_SIZE 20
#define NAME_LENGTH 8

typedef struct
{
	char board[9];
	char turn;
	char winner;
} game;

typedef struct
{
	int fd;
	char *name;
	int active;
	int opponent_idx;
	game *game;
	char symbol;
} client;

typedef enum 
{
	CONNECT,
	CONNECT_FAILED,
	PING,
	WAIT,
	GAME_FOUND,
	MOVE,
	GAME_FINISHED,
	DISCONNECT,
	EMPTY
} message_type;

typedef struct
{
	message_type message_type;
	game game;
	char name[NAME_LENGTH];
} message;

void empty_game_board(game* game);

void error_exit(char *msg);

void send_message(int fd, message_type type, game *game, char *name);

message receive_message(int fd);

message receive_message_nonblock(int fd);

#endif //COMMON_H