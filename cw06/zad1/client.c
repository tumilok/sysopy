#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/msg.h>

#include "common.h"

int client_q = -1;
int server_q = -1;
int chatting_q = -1;
int chatting_id = -1;
int client_id = -1;

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

void delete_queue()
{
    (msgctl(client_q, IPC_RMID, 0) == -1) ? 
        error("couldn't delete client queue"):
        printf("Client queue has been deleted\n");
}

void send_msg(int queue, int sender_id, int receiver_id, char *msg, int type) 
{
    msgbuf msg_buf;
    msg_buf.type = type;
    msg_buf.sender_id = sender_id;
    msg_buf.receiver_id = receiver_id;
    if (msg != NULL)
    {
        strcpy(msg_buf.text, msg);
    }

    if(msgsnd(queue, &msg_buf, msgbuf_size, 0) == -1)
    {
		delete_queue();
		error("client couldn't send message");
	}
}

void disconnect()
{
	send_msg(server_q, client_id, chatting_id, NULL, DISCONNECT);

	msgbuf msg_buf;
	if (msgrcv(client_q, &msg_buf, msgbuf_size, 0, 0) == -1 || msg_buf.type != DISCONNECT)
    {
        delete_queue();
		error("couldn't receive DISCONNECT message from server");
	}

	chatting_id = -1;
	chatting_q = -1;

	printf("Disconnected\n");
}

void stop()
{
    if (chatting_q != -1)
    {
		printf("Disconnecting...\n");
		disconnect();
	}

    if (server_q != -1) 
    {
        send_msg(server_q, client_id, -1, NULL, STOP);
    
        msgbuf msg_buf;
        if (msgrcv(client_q, &msg_buf, msgbuf_size, 0, 0) == -1 || msg_buf.type != STOP)
        {
            delete_queue();
            error("couldn't receive STOP message from server");
        }
        printf("Succesfully deleted from server\n");
    }

	if (client_q != -1){
        delete_queue();
    }
    printf("Client shutting down...\n");
}

void sigint_handler(int signal)
{
    printf("\n");
    exit(EXIT_SUCCESS);
}

void list()
{
    send_msg(server_q, client_id, -1, NULL, LIST);

    msgbuf msg_buf;
	if (msgrcv(client_q, &msg_buf, msgbuf_size, 0, 0) == -1 || msg_buf.type != LIST)
    {
        delete_queue();
		error("couldn't receive LIST message from server");
	}

    printf("%s", msg_buf.text);
}

void connect_to(int id, char *key)
{
    chatting_id = id;

	char* ptr;
	key_t chatting_key = strtol(key, &ptr, 10);

	chatting_q = msgget(chatting_key, 0);

	printf("Connected to %d\n", id);
}

void connect(int id)
{
    send_msg(server_q, client_id, id, NULL, CONNECT);

    msgbuf msg_buf;
	if (msgrcv(client_q, &msg_buf, msgbuf_size, 0, 0) == -1)
    {
		printf("Failed receiving connect response\n");
		return;
	}

	connect_to(msg_buf.receiver_id, msg_buf.text);
}

void init(key_t client_key)
{
    char text[MAX_MSG_SIZE];
	sprintf(text, "\t%d", client_key);

    send_msg(server_q, getpid(), -1, text, INIT);

    msgbuf msg_buf;
	if (msgrcv(client_q, &msg_buf, msgbuf_size, 0, 0) == -1 || msg_buf.type != INIT)
    {
        delete_queue();
		error("couldn't receive INIT message from server");
	}

	client_id = atoi(msg_buf.text);
	printf("Client with id: %d has successfully logged in\n", client_id);
}

void send_chat_message(char* text){

	msgbuf chat_msg;
	chat_msg.type = CHAT;
	chat_msg.sender_id = client_id;
	chat_msg.receiver_id = chatting_id;
	strcpy(chat_msg.text, text);

	if(msgsnd(chatting_q, &chat_msg, msgbuf_size, 0) == -1)
    {
		printf("Could not send message to %d", chatting_id);
		return;
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
    printf("server queue %d\n", server_q);

    init(client_key);

    char msg[MAX_MSG_SIZE];
	while (1)
    {
		msgbuf message;
        while(msgrcv(client_q, &message, msgbuf_size, 0, IPC_NOWAIT) >= 0)
        {
            switch (message.type)
            {
                case STOP:
                    server_q = -1;
                    exit(EXIT_SUCCESS);
                case DISCONNECT:
                    chatting_q = -1;
                    chatting_id = -1;
                    printf("Disconnected\n");
                    break;
                case CONNECT:
                    connect_to(message.receiver_id, message.text);
                    break;
                case CHAT:
                    printf("%s", message.text);
                    break;
            }
        }

        fgets(msg, sizeof msg, stdin);
        if(strcmp(msg, "") == 0) continue;

		if(strncmp(msg, list_str, strlen(list_str)) == 0)
        {
			list();
		}
		else if(strncmp(msg, connect_str, strlen(connect_str)) == 0 )
        {
			char* ptr;
			int id = strtol(msg + strlen(connect_str), &ptr, 10);
			connect(id);
		}
		else if(strncmp(msg, disconnect_str, strlen(disconnect_str)) == 0)
        {
			if(chatting_q == -1)
            {
				printf("Cannot disconnect if you are not connected\n");
				continue;
			}
			disconnect();
		}
		else if(strncmp(msg, stop_str, strlen(stop_str)) == 0)
        {
			exit(EXIT_SUCCESS);
		}
		else if(chatting_id != -1)
        {
			send_chat_message(msg);
		}
		else{
			printf("Invalid entry. Use command or if you're connected just chat! \n Available commands:\n%s\n%s\n%s (only when connected)\n%s\n",
					list_str, connect_str, disconnect_str, stop_str);
		}
	}
}