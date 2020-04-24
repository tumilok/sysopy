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

void handle_disconnect()
{
	printf("In disconnect with chatting queue: %d\n", chatting_q);
	if(mq_close(chatting_q) == -1){
		printf("Could not close chatting queue. Error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	chatting_q = -1;
	strcpy(chatting_name, "");
	printf("Disconnected\n");
}


void disconnect(){
	erase_nonblock();
	char message[MAX_MSG_SIZE];
	sprintf(message, "%d", client_id);
	unsigned int priority = DISCONNECT;
	if(mq_send(server_q, message, sizeof message, priority) == -1){
		printf("Could not send DISCONNECT to server. Error: %s\n", strerror(errno));
		return;
	}
	if(mq_receive(client_q, message, sizeof message, &priority) == -1){
		printf("Could not receive DISCONNECT response from server. Error: %s\n", strerror(errno));
		return;
	}
	handle_disconnect();
}


void handle_exit(){

	if(server_q != -1){
		if(mq_close(server_q) == -1) printf("Could not close server queue\n");
		else printf("Server queue closed\n");
	}

	if(client_q != -1){
		if(mq_close(client_q) == -1) printf("Could not close client queue. Error %s\n", strerror(errno));
		else printf("Client queue closed\n");

		if(mq_unlink(client_name) == -1) printf("Could not delete client queue\n");
		else printf("Client queue deleted\n");
	}

}

void exit_function()
{
	erase_nonblock();

	char message[MAX_MSG_SIZE];
	unsigned int priority = STOP;
	sprintf(message, "%d", client_id);

	if(chatting_q != -1){
		disconnect();
	}

	if(server_q != -1 && client_id != -1)
	{
		if(mq_send(server_q, message, sizeof message, priority) == -1){
			printf("Could not send STOP to server. Error: %s\n", strerror(errno));
			return;
		}
		else{
			printf("Request STOP sent to server\n");
		}
		if(mq_receive(client_q, message, sizeof message, &priority) == -1){
			printf("Could not receive STOP from server. Error: %s\n", strerror(errno));
			return;
		}
	}
	handle_exit();
	printf("\n=== EXIT ===\n");	
}


void set_queue_name(){

	srand(time(NULL));
	client_name[0] = '/';

	char a = 'a';
	char z = 'z';

	int range = (int) z - (int) a;

	for(int i = 1; i < NAME_LEN - 1; i++){
		client_name[i] = (char) ((int) a + rand() % range);
	}

	client_name[NAME_LEN - 1] = '\0';

}


void init(){

	char message[MAX_MSG_SIZE];
	unsigned int priority = INIT;
	strcpy(message, client_name);

	if(mq_send(server_q, message, sizeof message, priority) == -1){
		printf("Could not send INIT response to server\n");
		exit(EXIT_FAILURE);
	}

	if(mq_receive(client_q, message, sizeof message, &priority) == -1){
		printf("Could not receive INIT response from server\n");
		exit(EXIT_FAILURE);
	}

	if (priority == STOP)
	{
		error("server is full");
	}

	char* ptr;
	client_id = strtol(message, &ptr, 10);

	printf("Initialized with ID %d\n", client_id);
}



void list()
{
	erase_nonblock();

	char message[MAX_MSG_SIZE];
	unsigned int priority = LIST;
	sprintf(message, "%d", client_id);

	if(mq_send(server_q, message, sizeof message, priority) == -1){
		printf("Could not send LIST to server. Error: %s\n", strerror(errno));
		return;
	}
	else{
		printf("Request LIST sent to server\n");
	}
	if(mq_receive(client_q, message, sizeof message, &priority) == -1){
		printf("Could not receive LIST from server. Error: %s\n", strerror(errno));
		return;
	}

	printf("%s", message);

}

void handle_connect(char* message){
	if(strcmp(message, "") == 0){
		printf("Connecting failed\n");
		return;
	}
	strcpy(chatting_name, message);

	if((chatting_q = mq_open(message, O_WRONLY)) == -1){
		printf("Could not open chatting queue\n");
		return;
	}
	printf("Connected to %s\n", message);
}

void connect(int sec_id){
	erase_nonblock();

	char message[MAX_MSG_SIZE];
	sprintf(message, "%d %d", client_id, sec_id);
	unsigned int priority = CONNECT;
	if(mq_send(server_q, message, sizeof message, priority) == -1){
		printf("Could not send CONNECT to server. Error: %s\n", strerror(errno));
		return;
	}
	if(mq_receive(client_q, message, sizeof message, &priority) == -1){
		printf("Could not receive CONNECT response from server. Error: %s\n", strerror(errno));
		return;
	}

	handle_connect(message);
}

void send_chat_message(char* line){
	printf("Send: %s\n",line);
	char message[MAX_MSG_SIZE];
	strcpy(message, line);
	if(mq_send(chatting_q, message, sizeof message, CHAT) == -1){
		printf("Could not send CHAT message. Error: %s\n", strerror(errno));
	}

}

void receive_next_msg()
{
	apply_nonblock();

	char msg[MAX_MSG_SIZE];
	strcpy(msg, "");
	unsigned int priority;

	while(mq_receive(client_q, msg, sizeof msg, &priority) >= 0)
	{
		switch (priority)
		{
			case STOP:
				if (server_q != -1)
				{
					(mq_close(server_q) == -1) ? 
					error("could't close server queue"):
					printf("Server queue closed\n");
				}
				server_q = -1;
				exit(EXIT_SUCCESS);
			case DISCONNECT:
				handle_disconnect();
				break;
			case CONNECT:
				handle_connect(msg);
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
			send_chat_message(msg);
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
	atexit(exit_function);

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