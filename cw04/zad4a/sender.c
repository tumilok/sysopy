#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int recieved_signals = 0;
int sig2 = 0;
int is_q = 0;

void sigusr1_handler(int sig_no)
{
    recieved_signals++;
}

void sigusr2_handler(int sig_no, siginfo_t *info, void *ucontext)
{
    if (is_q == 1)
    {
        printf("\nCatcher received %d signals.\n", info -> si_int);
    }
    sig2 = 1;
}

void q_sigusr1_handler(int sig_no, siginfo_t *info, void *ucontext)
{
    printf("Recieved signal, next is %d\n", info -> si_int);
    recieved_signals++;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        // pid of catcher process, number of signals to send, flag of sending
        printf("Error, wrong number of initial parameters\n");
        exit(EXIT_FAILURE);
    }

    int catcher_pid = atoi(argv[1]);
    int signals_number = atoi(argv[2]);
    char *flag = argv[3];

    if (strcmp(flag, "kill") == 0)
    {
        struct sigaction act1;
        act1.sa_handler = sigusr1_handler;
        act1.sa_flags = 0;
        sigemptyset(&act1.sa_mask);
        sigaddset(&act1.sa_mask, SIGUSR1);
        sigaction(SIGUSR1, &act1, NULL);

        struct sigaction act2;
        act2.sa_sigaction = sigusr2_handler;
        act2.sa_flags = SA_SIGINFO;
        sigemptyset(&act2.sa_mask);
        sigaddset(&act2.sa_mask, SIGUSR2);
        sigaction(SIGUSR2, &act2, NULL);
        
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        for (int i = 0; i < signals_number; i++)
        {
            kill(catcher_pid, SIGUSR1);
        }

        kill(catcher_pid, SIGUSR2);
    }
    else if (strcmp(flag, "sigqueue") == 0)
    {
        is_q = 1;

        struct sigaction act1;
        act1.sa_sigaction = q_sigusr1_handler;
        act1.sa_flags = SA_SIGINFO;
        sigemptyset(&act1.sa_mask);
        sigaddset(&act1.sa_mask, SIGUSR1);
        sigaction(SIGUSR1, &act1, NULL);

        struct sigaction act2;
        act2.sa_sigaction = sigusr2_handler;
        act2.sa_flags = SA_SIGINFO;
        sigemptyset(&act2.sa_mask);
        sigaddset(&act2.sa_mask, SIGUSR2);
        sigaction(SIGUSR2, &act2, NULL);  

        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        union sigval value;
        value.sival_int = 0;

        for (int i = 0; i < signals_number; i++)
        {
            sigqueue(catcher_pid, SIGUSR1, value);
        }

        sigqueue(catcher_pid, SIGUSR2, value);
    }
    else if (strcmp(flag, "sigrt") == 0)
    {
        struct sigaction act1;
        act1.sa_handler = sigusr1_handler;
        act1.sa_flags = 0;
        sigemptyset(&act1.sa_mask);
        sigaddset(&act1.sa_mask, SIGRTMIN);
        sigaction(SIGRTMIN, &act1, NULL);

        struct sigaction act2;
        act2.sa_sigaction = sigusr2_handler;
        act2.sa_flags = SA_SIGINFO;
        sigemptyset(&act2.sa_mask);
        sigaddset(&act2.sa_mask, SIGRTMAX);
        sigaction(SIGRTMAX, &act2, NULL);  

        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMAX);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        for (int i = 0; i < signals_number; i++)
        {
            kill(catcher_pid, SIGRTMIN);
        }

        kill(catcher_pid, SIGRTMAX);
    }
    else
    {
        printf("Error, wrong parameter: %s\n", flag);
        exit(EXIT_FAILURE);
    }

    while (sig2 == 0)
        {
            pause();
        }
        
        printf("Sender recieved %d signals\n", recieved_signals);
        printf("Sender should had recieved %d signals\n", signals_number);

    return 0;
}