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

void send_msg(int queue, char *msg, int mtype) 
{
    msgbuf msg_buf;    
    msg_buf.mtype = mtype;
    msg_buf.sender_id = getpid();
    if (msg != NULL)
    {
        strcpy(msg_buf.msg, msg);
    }
    if (msgsnd(queue, &msg_buf, msgbuf_size, 0) == -1)
    {
        error("server couldn't send message");
    }
}

void stop_client(int client_id)
{
    send_msg(msgget(clients[client_id].queue_key, 0),
            msg_to_string(client_id), STOP);
    clients[client_id].pid = -1;
    clients[client_id].queue_key = -1;
    clients[client_id].chatting_id = -1;
    printf("Client with id: %d has loged out\n", client_id);
}

void disconnect(int client1_id)
{
    int client2_id = clients[client1_id].chatting_id;
    send_msg(msgget(clients[client1_id].queue_key, 0), NULL, DISCONNECT);
    clients[client1_id].chatting_id = -1;
    if (client1_id != client2_id)
    {
        send_msg(msgget(clients[client2_id].queue_key, 0), NULL, DISCONNECT);
        clients[client2_id].chatting_id = -1;
    }
    printf("Connection aborted %d <|=|> %d\n", client1_id, client2_id);
}

void list(int client_id)
{
    char msg[MAX_MSG_SIZE];
	char buff[MAX_MSG_SIZE];
	strcpy(buff, "\0");
	strcpy(msg, "--------------------\n");

	for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
		if (clients[i].pid != -1)
        {
		    char* status = (clients[i].chatting_id == -1) ? "free" : "busy";

		    sprintf(buff, "client %d\t%s\n", i, status);
		    strcat(msg, buff);
        }
	}
    strcat(msg, "--------------------\n");
    send_msg(msgget(clients[client_id].queue_key, 0), msg, LIST);
    printf("List was sent to client %d\n", client_id);
}

void send_connect_msg(int client_id, int chatting_id)
{
    clients[client_id].chatting_id = chatting_id;

    char *msg = msg_to_string(chatting_id);
    strcat(msg, " ");
    strcat(msg, msg_to_string(clients[chatting_id].queue_key));

    send_msg(msgget(clients[client_id].queue_key, 0), msg, CONNECT);
}

void connect(int client1_id, int client2_id)
{
    if (clients[client1_id].chatting_id != -1 || clients[client2_id].chatting_id != -1)
    {
		printf("Someone is still chatting");
        send_msg(msgget(clients[client1_id].queue_key, 0), NULL, STOP);
        return;
    }
    send_connect_msg(client1_id, client2_id);
    send_connect_msg(client2_id, client1_id);
    printf("Connection initialized %d <=> %d\n", client1_id, client2_id);
}

void init(msgbuf msg_buf)
{
    int id = -1;
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
		if (clients[i].pid == -1)
        {
            id = i;
            break;
        }
	}
    if (id == -1)
    {
        printf("Server is full\n");
        send_msg(msgget(atoi(msg_buf.msg), 0), NULL, STOP);
        return;
    }
    clients[id].pid = msg_buf.sender_id;
	clients[id].queue_key = atoi(msg_buf.msg);

    send_msg(msgget(clients[id].queue_key, 0), msg_to_string(id), INIT);
    printf("Client with id: %d has just logged in\n", id);
}

void terminate_server()
{
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
        if (clients[i].pid != -1)
        {
            stop_client(i);
        }
    }

    if (msgctl(server_q, IPC_RMID, 0) == -1)
    {
        error("couldnt delete server");
    }
    printf("\nServer was terminated\n");
}

void run_server()
{
    msgbuf msg_buf;
    while (1)
    {
        if (msgrcv(server_q, &msg_buf, msgbuf_size, 0, MSG_NOERROR) >= 0)
        {
            switch (msg_buf.mtype)
            {
            case STOP:
                stop_client(msg_buf.sender_id);
                break;
            case DISCONNECT:
                disconnect(msg_buf.sender_id);
                break;
            case LIST:
                list(msg_buf.sender_id);
                break;
            case CONNECT:
                connect(msg_buf.sender_id, atoi(msg_buf.msg));
                break;
            case INIT:
                init(msg_buf);
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    atexit(terminate_server);

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        error("couldn't install SIGINT handler");
    }

    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
        clients[i].pid = -1;
        clients[i].queue_key = -1;
        clients[i].chatting_id = -1;
    }

    key_t server_key = ftok(getenv("HOME"), SERVER_ID);
    if ((server_q = msgget(server_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }
    printf("Server was successfully initialized\n");

    run_server();
}