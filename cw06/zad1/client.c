#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/msg.h>

#include "common.h"

int client_queue = -1;
int server_queue = -1;
int chat_queue = -1;
int client_id = -1;


void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

void delete_queue()
{
    (msgctl(client_queue, IPC_RMID, 0) == -1) ? 
        error("couldn't delete client queue"):
        printf("Client queue has been deleted\n");
}

void sigint_handler(int signal)
{

    exit(EXIT_SUCCESS);
}

void send_msg(int queue, char *msg, int type) 
{
    msgbuf msg_buf;
    msg_buf.type = type;
    strcpy(msg_buf.text, msg);
    msg_buf.sender_id = getpid();
    msg_buf.reveiver_id = -1;
    if (msgsnd(queue, &msg_buf, sizeof(msg_buf.text), 0) == -1)
    {
        delete_queue();
        error("client couldn't send message");
    }
}

msgbuf *receive_msg(int queue) 
{
    msgbuf *msg_buf = calloc(1, sizeof(msgbuf));
    msg_buf -> type = 1;
    if (msgrcv(queue, msg_buf, sizeof(msg_buf -> text), 0, MSG_NOERROR))
    {
        delete_queue();
        error("client couldn't receive message");
    }
    return msg_buf;
}

void stop()
{

}

void disconnect()
{

}

void list()
{

}

void connect()
{

}

void init(key_t client_key)
{
    char s_client_key[MAX_MSG_SIZE];
    sprintf(s_client_key, "%d", client_key);
    send_msg(server_queue, s_client_key, INIT);
}

void chat()
{

}

int main(int argc, char *argv[])
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        error("couldn't install SIGINT handler");
    }

    key_t client_key = ftok(getenv("HOME"), getpid());
    if ((client_queue = msgget(client_key, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }

    key_t server_key = ftok(getenv("HOME"), SERVER_ID);
    if ((server_queue = msgget(server_queue, IPC_CREAT | 0777)) == -1)
    {
        error("couldn't create client queue");
    }

    init(client_key);

}