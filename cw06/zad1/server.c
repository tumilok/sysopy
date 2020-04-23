#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/msg.h>

#include "common.h"

typedef struct
{
    pid_t pid;
    key_t queue_key;
    int chatting_id;
} client;

client clients[MAX_CLIENTS_NUMBER];
int server_q = -1;

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
    strcpy(msg_buf.msg, msg);
    msg_buf.sender_id = getpid();
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

//////////////////////////////////////////////////////////

void send_disconnect_response(int client_id){

	msgbuf response;
	response.sender_id = getpid();
	//response.receiver_id = client_id;
	response.mtype = DISCONNECT;

	int c_queue_key = msgget(clients[client_id].queue_key, 0);

	if(msgsnd(c_queue_key, &response, msgbuf_size, 0) == -1){
		printf("Disconnecting %d failed", client_id);
		return;
	}

	clients[client_id].chatting_id = -1;
}

void disconnect(int client_id)
{
    int chatting_id = clients[client_id].chatting_id;

	send_disconnect_response(client_id);
	send_disconnect_response(chatting_id);
}

void list(int client_id)
{
    char* available = "AVAILABLE";
	char* busy = "BUSY";
    char msg[MAX_MSG_SIZE];

	char text_buf[MAX_MSG_SIZE];
	strcpy(text_buf, "\0");
	strcpy(msg, "\0");

	for(int i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
		if(clients[i].pid != -1)
        {
		    char* availability = (clients[i].chatting_id == -1) ? available : busy;

		    sprintf(text_buf, "ID: %d\tAVAILABILITY:\t%s\n", i, availability);
		    strcat(msg, text_buf);
        }
	}
    int client_q = msgget(clients[client_id].queue_key, 0);
    send_msg(client_q, msg, LIST);
}

void send_connect_msg(int client_id, int chatting_id){

	key_t chatting_key = clients[chatting_id].queue_key;

	msgbuf connect_msg;
	connect_msg.sender_id = client_id;
	//connect_msg.receiver_id = chatting_id;
	connect_msg.mtype = CONNECT;
	sprintf(connect_msg.msg, "%d", chatting_key);

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
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++)
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
	clients[id].queue_key = strtol(msg_buf.msg, &ptr, 10);
    
    int client_q = msgget(clients[id].queue_key, 0);
    
    char msg[MAX_MSG_SIZE];
    sprintf(msg, "%d", id);

    send_msg(client_q, msg, INIT);
    printf("Client with id: %d has just logged in\n", id);
}

void terminate_server()
{
    printf("\nTerminating server..\n");
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
    printf("server queue %d\n", server_q);

    printf("Server was successfully initialized\n");

    msgbuf msg_buf;

    while (1)
    {
        if (msgrcv(server_q, &msg_buf, msgbuf_size, 0, MSG_NOERROR) >= 0)
        {
            switch (msg_buf.mtype)
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
                //connect(msg_buf.sender_id, msg_buf.receiver_id);
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