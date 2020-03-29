#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void pending()
{
    sigset_t mask;
    sigpending(&mask);

    if (sigismember(&mask, SIGUSR1) == 1)
    {
        printf("signal SIGUSR1 is visible\n");
    }
    else
    {
        printf("signal SIGUSR1 is not visible\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error, wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }

    printf("Exec child process: ");

    if (strcmp(argv[1], "pending") == 0)
    {
        pending();
        return 0;
    }

    raise(SIGUSR1);

    if (strcmp(argv[1], "ignore") == 0)
    {  
        printf("signal SIGUSR1 is ignored\n");
    }
    else if (strcmp(argv[1], "handler") == 0)
    {

    }
    else if (strcmp(argv[1], "mask") == 0)
    {
        printf("signal SIGUSR1 is blocked\n");
    }
    else
    {
        printf("\nError, unknown option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}