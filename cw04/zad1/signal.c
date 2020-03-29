#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int sigtstp_flag;           // -1 paused, 1 running

void sigint_handler(int signum)
{
    printf("\nRecived signal SIGING\n");
    exit(0);
}

void sigtstp_handler(int sig_no)
{
    if (sigtstp_flag == 1)
    {
        printf("\nWaiting on CTRL+Z to continue or CTR+C to exit program\n");
    }
    else 
    {
        printf("\n");
    }
    sigtstp_flag = -sigtstp_flag;
}

int main(int argc, char *argv[])
{
    sigtstp_flag = 1;
    signal(SIGINT, sigint_handler);

    struct sigaction act;
    act.sa_handler = sigtstp_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGTSTP, &act, NULL);

    while(1)
    {
        if (sigtstp_flag == -1)
        {
            pause();
        }
        system("ls");
        sleep(3);
    }

    return 0;
}