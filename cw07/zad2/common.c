#include "common.h"

void error(char *msg)
{
	printf("%s Error: %s\n", msg, strerror(errno));
	exit(EXIT_FAILURE);
}

void sigint_handler(int signal)
{
	exit(EXIT_SUCCESS);
}