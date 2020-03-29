#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>


void handler(int signum)
{
    printf("handler recived SIGUSR1: %d\n", signum);
}

void pending()
{
    sigset_t mask;
    sigpending(&mask);

    if (sigismember(&mask, SIGUSR1))
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
        printf("Error, wrong number of parameters");
        exit(EXIT_FAILURE);
    }

    printf("\nParent process: ");

    if (strcmp(argv[1], "ignore") == 0)
    {
        signal(SIGUSR1, SIG_IGN);
        raise(SIGUSR1);
        printf("signal SIGUSR1 is ignored\n");

        if(fork() == 0)
        {
            raise(SIGUSR1);
            printf("Child process: signal SIGUSR1 is ignored\n");
        }
        else
        {
            if(fork() == 0)
            {
                execl("./child", "./child", argv[1], NULL);
            }
        }
    }
    else if (strcmp(argv[1], "handler") == 0)
    {
        signal(SIGUSR1, handler);
        raise(SIGUSR1);

        if(fork() == 0)
        {
            raise(SIGUSR1);
        }
        else
        {
            if(fork() == 0)
            {
                execl("./child", "./child", argv[1], NULL);
            }
        }
    }
    else if (strcmp(argv[1], "mask") == 0)
    {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);

        raise(SIGUSR1);
        printf("signal SIGUSR1 is blocked\n");

        if(fork() == 0)
        {
            printf("Child process: ");
            raise(SIGUSR1);
            printf("signal SIGUSR1 is blocked\n");
        }
        else
        {
            if(fork() == 0)
            {
                execl("./child", "./child", argv[1], NULL);
            }
        }
    }
    else if (strcmp(argv[1], "pending") == 0)
    {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);

        raise(SIGUSR1);
        pending();

        if(fork() == 0)
        {
            printf("Child process: ");
            pending();
        }
        else
        {
            if(fork() == 0)
            {
                execl("./child", "./child", argv[1], NULL);
            }
        }
    }
    else
    {
        printf("\nError, unknown option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}