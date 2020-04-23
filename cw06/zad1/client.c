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

void send_msg(int queue, char *msg, int mtype) 
{
    msgbuf msg_buf;
    msg_buf.mtype = mtype;
    msg_buf.sender_id = (mtype == INIT) ? getpid() : client_id;
    if (msg != NULL)
    {
        strcpy(msg_buf.msg, msg);
    }

    if(msgsnd(queue, &msg_buf, msgbuf_size, 0) == -1)
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

///////////////////////////////////////////

void disconnect()
{
	send_msg(server_q, msg_to_string(chatting_id), DISCONNECT);

	msgbuf msg_buf;
	if (msgrcv(client_q, &msg_buf, msgbuf_size, 0, 0) == -1 || msg_buf.mtype != DISCONNECT)
    {
        delete_queue();
		error("couldn't receive DISCONNECT message from server");
	}

	chatting_id = -1;
	chatting_q = -1;

	printf("Disconnected\n");
}

////////////////////////////////////////////////////////////////
//DONE

void stop()
{
    if (chatting_q != -1)
    {
		printf("Disconnecting from chatbox...\n");
		disconnect();
	}
    if (server_q != -1 && client_id != -1) 
    {
        send_msg(server_q, NULL, STOP);

        msgbuf *response = receive_msg(client_q);
        if (response -> mtype != STOP)
        {
            delete_queue();
            error("expected to receive STOP message from server");
        }
        printf("Succesfully logged out from the server\n");
    }
	if (client_q != -1)
    {
        delete_queue();
    }
}

void list()
{
    send_msg(server_q, NULL, LIST);
    msgbuf *response = receive_msg(client_q);
    if (response -> mtype != LIST)
    {
        delete_queue();
        error("expected to receive LIST message from server");
    }
    printf("%s", response -> msg);
}

////////////////////////////////////////////////////////////////////

void connect_to(char *msg)
{
    int id = atoi(strtok(msg, " "));
    int key = atoi(strtok(NULL, " "));

    printf("id: %d, key: %d\n", id, key);
    chatting_id = id;
	chatting_q = msgget(key, 0);
	printf("Connected to %d\n", id);
}

void connect(char *id)
{
    send_msg(server_q, id, CONNECT);
    msgbuf *response = receive_msg(client_q);
    if (response -> mtype != CONNECT)
    {
        delete_queue();
        error("expected to receive CONNECT message from server");
    }
	connect_to(response -> msg);
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

void send_chat_message(char* msg){

	msgbuf chat_msg;
	chat_msg.mtype = CHAT;
	chat_msg.sender_id = client_id;
	strcpy(chat_msg.msg, msg);

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

    init(client_key);

    char msg[MAX_MSG_SIZE];
	while (1)
    {
		msgbuf message;
        while(msgrcv(client_q, &message, msgbuf_size, 0, IPC_NOWAIT) >= 0)
        {
            switch (message.mtype)
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
                    connect_to(message.msg);
                    break;
                case CHAT:
                    printf("%s", message.msg);
                    break;
            }
        }
        printf("[client %d]$ ", client_id);
        fgets(msg, sizeof msg, stdin);
        if(strcmp(msg, "") == 0) continue;

		if(strncmp(msg, list_str, strlen(list_str)) == 0)
        {
			list();
		}
		else if(strncmp(msg, connect_str, strlen(connect_str)) == 0 )
        {
			connect(msg + strlen(connect_str));
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