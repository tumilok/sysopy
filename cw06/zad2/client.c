#include "client.h"

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

void apply_nonblock()
{
	struct mq_attr new, old;
	new.mq_flags = O_NONBLOCK;
	mq_getattr(client_q, &old);
	mq_setattr(client_q, &new, &old);
	mq_getattr(client_q, &old);
}

void erase_nonblock()
{
	struct mq_attr new, old;
	new.mq_flags = 0;
	mq_getattr(client_q, &old);
	mq_setattr(client_q, &new, &old);
	mq_getattr(client_q, &old);
}

void set_queue_name()
{
	srand(time(NULL));
	client_name[0] = '/';

	char a = 'a';
	char z = 'z';

	int range = (int) z - (int) a;

	for(int i = 1; i < NAME_LEN - 1; i++)
	{
		client_name[i] = (char) ((int) a + rand() % range);
	}
	client_name[NAME_LEN - 1] = '\0';
}

void expected_type(const char *exp_str, long exp, long got)
{
    if (exp != got)
    {
        char *error_msg = "Wrong type of message, expected ";
        strcat(error_msg, exp_str);
        error(error_msg);
    }
}

void send_msg(mqd_t queue, char *msg, int priority) 
{
	char to_send[MAX_MSG_SIZE];
	strcpy(to_send, msg);
	if (mq_send(queue, to_send, sizeof(to_send), priority) == -1)
	{
		error("client couldn't send message");
	}
}

void disconnect_handler()
{
	(mq_close(chatting_q) == -1) ?
		error("could't close server queue") :
		printf("Disconnected from chatbox\n");
	chatting_q = -1;
	strcpy(chatting_name, "");
}

void disconnect()
{
	erase_nonblock();

	char msg[MAX_MSG_SIZE];
	sprintf(msg, "%d", client_id);
	unsigned int priority = DISCONNECT;

	send_msg(server_q, msg, priority);
	if (mq_receive(client_q, msg, sizeof(msg), &priority) == -1)
	{
		error("couldn't receive DISCONNECT message");
	}
	expected_type(disconnect_str, DISCONNECT, priority);
	disconnect_handler();
}

void stop()
{
	erase_nonblock();

	char msg[MAX_MSG_SIZE];
	unsigned int priority = STOP;
	sprintf(msg, "%d", client_id);

	if (chatting_q != -1)
	{
		disconnect();
	}
	if (server_q != -1 && client_id != -1)
	{
		send_msg(server_q, msg, priority);
		if (mq_receive(client_q, msg, sizeof msg, &priority) == -1)
		{
			error("couldn't receive STOP message");
		}
		expected_type(stop_str, STOP, priority);

		(mq_close(server_q) == -1) ?
			error("couldn't close server queue") :
			printf("Server queue has been closed\n");
	}
	if (client_q != -1)
	{
		(mq_close(client_q) == -1) ?
			error("couldn't close client queue") :
			printf("Client queue has been closed\n");

		(mq_unlink(client_name) == -1) ?
			error("couldn't delete client queue") :
			printf("Client queue has been deleted\n");
	}
}

void init()
{
	char msg[MAX_MSG_SIZE];
	strcpy(msg, client_name);
	unsigned int priority = INIT;

	send_msg(server_q, msg, priority);
	if (mq_receive(client_q, msg, sizeof msg, &priority) == -1)
	{
		error("couldn't receive INIT message");
	}
	if (priority == STOP)
	{
		error("server is full");
	}
	client_id = atoi(msg);
	printf("Client got id: %d\n", client_id);
}

void list()
{
	erase_nonblock();

	char msg[MAX_MSG_SIZE];
	sprintf(msg, "%d", client_id);
	unsigned int priority = LIST;
	
	send_msg(server_q, msg, priority);
	if (mq_receive(client_q, msg, sizeof(msg), &priority) == -1)
	{
		error("couldn't receive LIST message");
	}
	expected_type(list_str, LIST, priority);
	printf("%s", msg);
}

void connect_handler(char* msg)
{
	if(!strcmp(msg, ""))
	{
		printf("Connecting failed\n");
		return;
	}
	strcpy(chatting_name, msg);

	if ((chatting_q = mq_open(msg, O_WRONLY)) == -1)
	{
		printf("Could not open chatting queue\n");
		return;
	}
	printf("Connected to %s\n", msg);
}

void connect(int client2_id)
{
	erase_nonblock();

	char msg[MAX_MSG_SIZE];
	sprintf(msg, "%d %d", client_id, client2_id);
	unsigned int priority = CONNECT;

	send_msg(server_q, msg, priority);
	if (mq_receive(client_q, msg, sizeof(msg), &priority) == -1)
	{
		error("client couldn't receive CONNECT message");
	}
	expected_type(connect_str, CONNECT, priority);

	connect_handler(msg);
}

void stop_handler()
{
	if (server_q != -1)
	{
		(mq_close(server_q) == -1) ? 
		error("could't close server queue"):
		printf("Server queue closed\n");
	}
	server_q = -1;
	exit(EXIT_SUCCESS);
}

void receive_next_msg()
{
	apply_nonblock();

	char msg[MAX_MSG_SIZE];
	unsigned int priority;
	while (mq_receive(client_q, msg, sizeof(msg), &priority) >= 0)
	{
		switch (priority)
		{
			case STOP:
				stop_handler();
				break;
			case DISCONNECT:
				disconnect_handler();
				break;
			case CONNECT:
				connect_handler(msg);
				break;
			case CHAT:
				printf("%s", msg);
				break;
		}
		received = 1;
	}
}

void run_client()
{
	char msg[MAX_MSG_SIZE];
	while (1)
	{
		receive_next_msg();

        printf("(%d): ", client_id);
        fgets(msg, sizeof msg, stdin);

		if (!strcmp(msg, "\n"))
		{

		}
		else if (!strncmp(msg, list_str, strlen(list_str)))
		{
			list();
		}
		else if (!strncmp(msg, connect_str, strlen(connect_str)))
		{
			connect(atoi(msg + strlen(connect_str)));
		}
		else if (!strncmp(msg, disconnect_str, strlen(disconnect_str)))
		{
			if (chatting_q == -1)
            {
				printf("Cannot disconnect if you are not connected\n");
				continue;
			}
			disconnect();
		}
		else if (!strncmp(msg, stop_str, strlen(stop_str)))
		{
			exit(EXIT_SUCCESS);
		}
		else if (chatting_q != -1)
		{
			send_msg(chatting_q, msg, CHAT);
		}
		else
		{
			printf("Unknown command\nAvailable commands:\n%s\n%s\n%s\n%s\n%s\n",
             "Send messages if you are in a chat mode or press enter to refresh",
					list_str, connect_str, disconnect_str, stop_str);
		}
	}
}

int main(int argc, char** argv)
{
	atexit(stop);

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        error("couldn't install SIGINT handler");
    }

	set_queue_name();

	struct mq_attr attr;

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;

	if ((client_q = mq_open(client_name, O_CREAT | O_EXCL | O_RDWR, 0666, &attr)) == -1)
	{
		error("couldn't create client queue");
	}

	if ((server_q = mq_open(S_QUEUE_NAME, O_WRONLY)) == -1)
	{
		error("couldn't create server queue");
	}

	init();
	run_client();
}