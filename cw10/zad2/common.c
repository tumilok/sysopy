#include "common.h"

void empty_game_board(game *game)
{
	for (int i = 0; i < 9; i++)
    {
		game->board[i] = '-';
	}
	game->turn = 'O';
	game->winner = '-';
}

void error_exit(char *msg)
{
	printf("Error, %s\n", msg);
	if (errno)
	{
		printf("%s\n", strerror(errno));
	}
	exit(EXIT_FAILURE);
}

void send_message(int fd, message_type type, game *game, char* name)
{
	char *message = calloc(MSG_SIZE, sizeof(char));

	if (game == NULL)
    {
        sprintf(message, "%d %s", (int) type, name);
    }
	else
    {
        sprintf(message, "%d %s %c %c %s", (int) type, game->board, game->turn, game->winner, name);
    }

	if (write(fd, message, MSG_SIZE) < 0)
    {
        error_exit("Could not send message.");
    }
	free(message);
}

void send_message_to(int fd, struct sockaddr* addr, message_type type, game *game, char *name)
{
	char *message = calloc(MSG_SIZE, sizeof(char));

	if (game == NULL)
    {
        sprintf(message, "%d %s", (int) type, name);
    }
	else
    {
        sprintf(message, "%d %s %c %c %s", (int) type, game -> board, game -> turn, game -> winner, name);
    }

	if (sendto(fd, message, MSG_SIZE, 0, addr, sizeof(struct sockaddr)) < 0)
    {
        error_exit("Could not send message.");
    }
	free(message);
}

message receive_message(int fd)
{
	message msg;
	int count;
	char *msg_buf = calloc(MSG_SIZE, sizeof(char));

	if ((count = read(fd, msg_buf, MSG_SIZE)) < 0)
    {
        error_exit("Could not receive message.");
    }
	if (count == 0)
    {
		msg.message_type = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char *token;
	char *rest = msg_buf;
	strcpy(msg.name, "");
	empty_game_board(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.message_type = (message_type) atoi(token);

	switch (msg.message_type)
    {
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		default:
			break;
	}
	free(msg_buf);
	return msg;
}

message receive_message_from(int fd, struct sockaddr *addr, socklen_t len)
{
	message msg;
	int count;
	char *msg_buf = calloc(MSG_SIZE, sizeof(char));

	if ((count = recvfrom(fd, msg_buf, MSG_SIZE, 0, addr, &len)) < 0)
    {
        error_exit("Could not receive message.");
    }
	if (count == 0)
    {
		msg.message_type = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char *token;
	char *rest = msg_buf;
	strcpy(msg.name, "");
	empty_game_board(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.message_type = (message_type) atoi(token);

	switch (msg.message_type)
    {
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			break;
		default:
			break;
	}
	free(msg_buf);
	return msg;
}

message receive_message_nonblock(int fd)
{
	message msg;
	char *msg_buf = calloc(MSG_SIZE, sizeof(char));
	int count;

	if ((count = recv(fd, msg_buf, MSG_SIZE, MSG_DONTWAIT)) < 0)
    {
		msg.message_type = EMPTY;
		free(msg_buf);
		return msg;
	}

	if (count == 0)
    {
		msg.message_type = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char *token;
	char *rest = msg_buf;
	char* p;
	strcpy(msg.name, "");
	empty_game_board(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.message_type = (message_type) strtol(token, &p, 10);

	switch (msg.message_type)
    {
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			break;
		default:
			break;
	}
	free(msg_buf);
	return msg;
}