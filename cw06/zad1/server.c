#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/msg.h>

#include "common.h"

struct client
{
    pid_t pid;
    key_t queue_key;
    int chatting_id;
};
typedef struct client client;

client clients[MAX_CLIENTS];
int server_queue = -1;

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

void send_msg(int queue, int receiver_id, char *msg, int type) 
{
    msgbuf msg_buf;    
    msg_buf.type = type;
    strcpy(msg_buf.text, msg);
    msg_buf.sender_id = getpid();
    msg_buf.receiver_id = receiver_id;
    if (msgsnd(queue, &msg_buf, msgbuf_size, 0) == -1)
    {
        error("server couldn't send message");
    }
}

void stop_client(int client_id)
{
    msgbuf stop_msg;
    stop_msg.type = STOP;
    stop_msg.sender_id = getpid();
    stop_msg.receiver_id = client_id;

    int client_q = msgget(clients[client_id].queue_key, 0);

    if (msgsnd(client_q, &stop_msg, msgbuf_size, 0) == -1)
    {
        error("couldn't stop client");
    }

    clients[client_id].pid = -1;
    clients[client_id].queue_key = -1;
    clients[client_id].chatting_id = -1;

    printf("Client %d removed\n", client_id);
}

void disconnect(int client_id)
{
    int client1_q;
    if ((client1_q = msgget(clients[client_id].queue_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }
    send_msg(client1_q, client_id, "", DISCONNECT);

    int client2_q;
    if ((client2_q = msgget(clients[clients[client_id].chatting_id].queue_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }
    send_msg(client2_q, client_id, NULL, DISCONNECT);

    clients[clients[client_id].chatting_id].chatting_id = -1;
    clients[client_id].chatting_id = -1;
}

void list(int client_id)
{
    char* available = "AVAILABLE";
	char* busy = "BUSY";
    char msg[MAX_MSG_SIZE];

	char text_buf[MAX_MSG_SIZE];
	strcpy(text_buf, "\0");
	strcpy(msg, "\0");

	for(int i = 0; i < MAX_CLIENTS; i++)
    {
		if(clients[i].pid != -1)
        {
		    char* availability = (clients[i].chatting_id == -1) ? available : busy;

		    sprintf(text_buf, "ID: %d\tAVAILABILITY:\t%s\n", i, availability);
		    strcat(msg, text_buf);
        }
	}
    printf("%d", client_id);
    int client_q = msgget(clients[client_id].queue_key, 0);
    send_msg(client_q, client_id, msg, LIST);
}

void send_connect_msg(int client_id, int chatting_id){

	key_t chatting_key = clients[chatting_id].queue_key;

	msgbuf connect_msg;
	connect_msg.sender_id = client_id;
	connect_msg.receiver_id = chatting_id;
	connect_msg.type = CONNECT;
	sprintf(connect_msg.text, "%d", chatting_key);

	int c_queue_key = msgget(clients[client_id].queue_key, 0);

	if(msgsnd(c_queue_key, &connect_msg, msgbuf_size, 0) == -1)
    {
		printf("Could not connect client of ID %d\n", client_id);
		return;
	}

	clients[client_id].chatting_id = chatting_id;

	printf("Conencted %d to %d", client_id, chatting_id);

}

void connect(int sender_id, int receiver_id)
{
    if(clients[sender_id].chatting_id != -1)
    {
		printf("Client of id %d is still chatting", sender_id);
		return;
	}
	if(clients[receiver_id].chatting_id != -1)
    {
		printf("Client of id %d is still chatting", receiver_id);
		return;
	}

	send_connect_msg(sender_id, receiver_id);
	send_connect_msg(receiver_id, sender_id);

	if(clients[sender_id].chatting_id == -1 || clients[receiver_id].chatting_id == -1)
    {
		clients[sender_id].chatting_id = clients[receiver_id].chatting_id = -1;
		printf("Connection between %d and %d failed\n", sender_id, receiver_id);
	}
}

void init(msgbuf msg_buf)
{
    int id = -1;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
		if (clients[i].pid == -1)
        {
            id = i;
        }
	}
    if (id == -1)
    {
        printf("Server is full\n");
        return;
    }

    clients[id].pid = msg_buf.sender_id;
    char* ptr;
	clients[id].queue_key = strtol(msg_buf.text, &ptr, 10);
    
    int client_q = msgget(clients[id].queue_key, 0);
    
    char msg[MAX_MSG_SIZE];
    sprintf(msg, "%d", id);

    send_msg(client_q, msg_buf.sender_id, msg, INIT);
    printf("Client with id: %d has just logged in\n", id);
}

void terminate_server()
{
    printf("\nTerminating server..\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].pid != -1)
        {
            stop_client(i);
        }
    }

    if (msgctl(server_queue, IPC_RMID, 0) == -1)
    {
        error("couldnt delete server");
    }
}

void sigint_handler(int signal)
{
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
    atexit(terminate_server);

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        error("couldn't install SIGINT handler");
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].pid = -1;
        clients[i].queue_key = -1;
        clients[i].chatting_id = -1;
    }

    key_t server_key = ftok(getenv("HOME"), SERVER_ID);
    if ((server_queue = msgget(server_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }
    printf("server queue %d\n", server_queue);

    printf("Server was successfully initialized\n");

    msgbuf msg_buf;

    while (1)
    {
        if (msgrcv(server_queue, &msg_buf, msgbuf_size, 0, MSG_NOERROR) >= 0)
        {
            switch (msg_buf.type)
            {
            case STOP:
                printf("Received STOP from client %d...\n", msg_buf.sender_id);
                stop_client(msg_buf.sender_id);
                break;
            case DISCONNECT:
                printf("Received DISCONNECT from client %d...\n", msg_buf.sender_id);
                disconnect(msg_buf.sender_id);
                break;
            case LIST:
                printf("Received LIST from client %d...\n", msg_buf.sender_id);
                list(msg_buf.sender_id);
                break;
            case CONNECT:
                printf("Received CONNECT from client %d...\n", msg_buf.sender_id);
                connect(msg_buf.sender_id, msg_buf.receiver_id);
                break;
            case INIT:
                printf("Received INIT from client %d...\n", msg_buf.sender_id);
                init(msg_buf);
                break;
            default:
                printf("Received UNKNOWN from client %d...\n", msg_buf.sender_id);
                break;
            }
        }
    }

}