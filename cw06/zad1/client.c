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

void delete_queue()
{
    (msgctl(client_q, IPC_RMID, 0) == -1) ? 
        error("couldn't delete client queue"):
        printf("Client queue has been deleted\n");
}

void expected_type(const char *exp_str, long exp, long got)
{
    if (exp != got)
    {
        char *error_msg = "Wrong type of message, expected ";
        strcat(error_msg, exp_str);

        delete_queue();
        error(error_msg);
    }
}

void send_msg(int queue, char *msg, int mtype) 
{
    msgbuf msg_buf;
    msg_buf.mtype = mtype;
    msg_buf.sender_id = (mtype == INIT) ? getpid() : client_id;
    if (msg != NULL)
    {
        strcpy(msg_buf.msg, msg);
    }

    if (msgsnd(queue, &msg_buf, msgbuf_size, 0) == -1)
    {
		delete_queue();
		error("client couldn't send message");
	}
}

msgbuf *receive_msg(int queue)
{
    msgbuf *msg_buf = calloc(1, sizeof(msgbuf));
    if (msgrcv(queue, msg_buf, msgbuf_size, 0, MSG_NOERROR) == -1)
    {
        delete_queue();
        error("couldn't receive message");
    }
    return msg_buf;
}

void receive_next_msg()
{
    msgbuf msg_buf;
    while (msgrcv(client_q, &msg_buf, msgbuf_size, 0, IPC_NOWAIT) >= 0)
    {
        switch (msg_buf.mtype)
        {
            case STOP:
                server_q = -1;
                exit(EXIT_SUCCESS);
            case DISCONNECT:
                chatting_q = -1;
                chatting_id = -1;
                printf("Disconnected from chatbox\n");
                break;
            case CONNECT:
                if (chatting_id == -1)
                {
                    connect_to(msg_buf.msg);
                }
                break;
            case CHAT:
                printf("%s", msg_buf.msg);
                break;
        }
    }
}

void stop()
{
    if (chatting_q != -1)
    {
		disconnect();
	}
    if (server_q != -1 && client_id != -1) 
    {
        send_msg(server_q, NULL, STOP);
        msgbuf *response = receive_msg(client_q);
        expected_type(stop_str, STOP, response -> mtype);
        printf("Succesfully logged out from the server\n");
    }
	if (client_q != -1)
    {
        delete_queue();
    }
}

void disconnect()
{
	send_msg(server_q, msg_to_string(chatting_id), DISCONNECT);
	msgbuf *response = receive_msg(client_q);
    expected_type(disconnect_str, DISCONNECT, response -> mtype);
	chatting_id = -1;
	chatting_q = -1;
	printf("Disconnected from chatbox\n");
}

void list()
{
    send_msg(server_q, NULL, LIST);
    msgbuf *response = receive_msg(client_q);
    expected_type(list_str, LIST, response -> mtype);
    printf("%s", response -> msg);
}

void connect_to(char *msg)
{
    int id = atoi(strtok(msg, " "));
    int key = atoi(strtok(NULL, " "));

    chatting_id = id;
	chatting_q = msgget(key, 0);
	printf("Connected to %d\n", id);
}

void connect(char *id)
{
    send_msg(server_q, id, CONNECT);
    msgbuf *response = receive_msg(client_q);
    if (response -> mtype == CONNECT)
    {
        connect_to(response -> msg);
    }
}

void init(key_t client_key)
{
    send_msg(server_q, msg_to_string(client_key), INIT);
    msgbuf *response = receive_msg(client_q);
    if (response -> mtype == STOP)
    {
        error("server is full");
    }
	client_id = atoi(response -> msg);
	printf("Client got id: %d\n", client_id);
}

void run_client()
{
    char msg[MAX_MSG_SIZE];
	while (1)
    {
		receive_next_msg();

        printf("(%d): ", client_id);
        fgets(msg, sizeof msg, stdin);

        if (!strcmp(msg, refresh_str))
        {

        }
		else if (!strncmp(msg, list_str, strlen(list_str)))
        {
			list();
		}
		else if (!strncmp(msg, connect_str, strlen(connect_str)))
        {
			connect(msg + strlen(connect_str));
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
		else if (chatting_id != -1)
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

int main(int argc, char *argv[])
{
    atexit(stop);

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        error("couldn't install SIGINT handler");
    }

    key_t client_key = ftok(getenv("HOME"), getpid());
    if ((client_q = msgget(client_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }

    key_t server_key = ftok(getenv("HOME"), SERVER_ID);
    if ((server_q = msgget(server_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }

    init(client_key);
    run_client();    
}