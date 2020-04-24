#include "server.h"

void sigint_handler(int signal)
{
    printf("\n");
    exit(EXIT_SUCCESS);
}

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

char *msg_to_string(int msg)
{
    char *buff = calloc(MAX_MSG_SIZE, sizeof(char));
    sprintf(buff, "%d", msg);
    return buff;
}

int get_index()
{
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
		if (clients[i].queue == -1)
        {
            return i;
        }
	}
	return -1;
}

void send_msg(mqd_t queue, char *msg, int priority) 
{
	char to_send[MAX_MSG_SIZE];
	strcpy(to_send, msg);
	if (mq_send(queue, to_send, sizeof(to_send), priority) == -1)
	{
		error("server couldn't send message");
	}
}

void close_queue(int client_id)
{
	mqd_t queue = (client_id == -1) ? server_q : clients[client_id].queue;
	if (mq_close(queue) == -1)
	{
		error("couldn't close client queue");
	}
}

void stop_client(int client_id)
{	
    send_msg(clients[client_id].queue, "", STOP);
	close_queue(client_id);

	clients[client_id].queue = -1;
	strcpy(clients[client_id].name, "");

    printf("Client with id: %d has loged out\n", client_id);
}

void disconnect(int client1_id)
{
	int client2_id = clients[client1_id].chatting_id;
    send_msg(clients[client1_id].queue, "", DISCONNECT);
    clients[client1_id].chatting_id = -1;
    if (client1_id != client2_id)
    {
        send_msg(clients[client2_id].queue, "", DISCONNECT);
        clients[client2_id].chatting_id = -1;
    }
    printf("Connection aborted %d <|=|> %d\n", client1_id, client2_id);
}

void list(int client_id)
{
	char msg[MAX_MSG_SIZE];
	char buff[MAX_MSG_SIZE];
	strcpy(buff, "");
	strcpy(msg, "--------------------\n");

	for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
	{
		if (clients[i].queue != -1)
		{
			char *status = (clients[i].chatting_id == -1) ? "free" : "busy";

			sprintf(buff, "client %d\t%s\n", i, status);
			strcat(msg, buff);
		}
	}
	send_msg(clients[client_id].queue, msg, LIST);
	printf("List was sent to client %d\n", client_id);
}

void send_connect_msg(int client1_id, int client2_id)
{
	char msg[MAX_MSG_SIZE];
	strcpy(msg, clients[client2_id].name);
	send_msg(clients[client1_id].queue, msg, CONNECT);
	clients[client1_id].chatting_id = client2_id;
}

void connect(char* msg)
{
	int client1_id = atoi(strtok(msg, " "));
	int client2_id = atoi(strtok(NULL, " "));

	if (clients[client1_id].chatting_id != -1 || clients[client2_id].chatting_id != -1)
    {
		printf("Someone is still chatting");
        send_msg(clients[client1_id].queue, "", CONNECT);
        return;
    }
    send_connect_msg(client1_id, client2_id);
    send_connect_msg(client2_id, client1_id);
    printf("Connection initialized %d <=> %d\n", client1_id, client2_id);
}

void init(char *msg, int priority)
{
	int id = get_index();
    if (id == -1)
    {
        printf("Server is full\n");
        send_msg(mq_open(msg, O_WRONLY), "", STOP);
        return;
    }

	strcpy(clients[id].name, msg);
	clients[id].queue = mq_open(clients[id].name, O_WRONLY);
	
	send_msg(clients[id].queue, msg_to_string(id), priority);
    printf("Client with id: %d has just logged in\n", id);
}

void terminate_server()
{
	for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
	{
		if (clients[i].queue != -1)
		{
			 stop_client(i);
		}
	}
	if (server_q != -1)
	{
		close_queue(-1);
	}
	if (mq_unlink(S_QUEUE_NAME) == -1)
	{
		error("couldn't delete server queue");
	}
}

void run_server()
{
    char msg_buf[MAX_MSG_SIZE];
	unsigned int priority;
	while (1)
	{
		if (mq_receive(server_q, msg_buf, sizeof msg_buf, &priority) == -1)
		{
			error("couldn't read message");
		}

		switch (priority)
		{
			case STOP:
				stop_client(atoi(msg_buf));
				break;
			case DISCONNECT:
				disconnect(atoi(msg_buf));
				break;
			case LIST:
				list(atoi(msg_buf));
				break;
			case CONNECT:
				connect(msg_buf);
				break;
			case INIT:
				init(msg_buf, priority);
				break;
		}
	}
}

int main(int argc, char** argv)
{
	atexit(terminate_server);

	if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        error("couldn't install SIGINT handler");
    }

	for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
	{
		strcpy(clients[i].name, "");
		clients[i].queue = -1;
		clients[i].chatting_id = -1;
	}

	struct mq_attr attr;

    attr.mq_flags = O_NONBLOCK;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;

	if ((server_q = mq_open(S_QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attr)) == -1)
	{
		error("couldn't create client queue");
	}
	printf("Server was successfully initialized\n");

	run_server();
}