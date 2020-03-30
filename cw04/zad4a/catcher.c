#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int recieved_signals = 0;
int sig2 = 0;
int sender_pid;

void sigusr1_handler(int sig_no)
{
    recieved_signals++;
}

void sigusr2_handler(int sig_no, siginfo_t *info, void *ucontext)
{
    sender_pid = info -> si_pid;
    sig2 = 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        // flag of sending
        printf("Error, wrong number of initial parameters\n");
        exit(EXIT_FAILURE);
    }
    char *flag = argv[1]; 

    printf("Catcher process PID: %d\n", getpid());

    if (strcmp(flag, "kill") == 0)
    {
        struct sigaction act1;
        act1.sa_handler = sigusr1_handler;
        act1.sa_flags = 0;
        sigemptyset(&act1.sa_mask);
        sigaddset(&act1.sa_mask, SIGUSR2);
        sigaction(SIGUSR1, &act1, NULL);

        struct sigaction act2;
        act2.sa_sigaction = sigusr2_handler;
        act2.sa_flags = SA_SIGINFO;
        sigemptyset(&act2.sa_mask);
        sigaddset(&act2.sa_mask, SIGUSR1);
        sigaction(SIGUSR2, &act2, NULL);
        
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        while (sig2 == 0)
        {
            pause();
        }

        printf("Catcher recieved %d signals\n", recieved_signals);

        for (int i = 0; i < recieved_signals; i++)
        {
            kill(sender_pid, SIGUSR1);
        }

        kill(sender_pid, SIGUSR2);
        
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(flag, "sigqueue") == 0)
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

        while (sig2 == 0)
        {
            pause();
        }

        printf("Catcher recieved %d signals\n", recieved_signals);

        union sigval value;
        for (int i = 0; i < recieved_signals; i++)
        {
            value.sival_int = i + 1;
            sigqueue(sender_pid, SIGUSR1, value);
        }
        value.sival_int = recieved_signals;
        sigqueue(sender_pid, SIGUSR2, value);

	exit(EXIT_SUCCESS);
    }
    else if (strcmp(flag, "sigrt") == 0)
    {
        struct sigaction act1;
        act1.sa_handler = sigusr1_handler;
        act1.sa_flags = 0;
        sigemptyset(&act1.sa_mask);
        sigaddset(&act1.sa_mask, SIGRTMAX);
        sigaction(SIGRTMIN, &act1, NULL);

        struct sigaction act2;
        act2.sa_sigaction = sigusr2_handler;
        act2.sa_flags = SA_SIGINFO;
        sigemptyset(&act2.sa_mask);
        sigaddset(&act2.sa_mask, SIGRTMIN);
        sigaction(SIGRTMAX, &act2, NULL);  

        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMAX);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        while (sig2 == 0)
        {
            pause();
        }

        printf("Catcher recieved %d signals\n", recieved_signals);

        for (int i = 0; i < recieved_signals; i++)
        {
            kill(sender_pid, SIGRTMIN);
        }

        kill(sender_pid, SIGRTMAX);
        
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Error, wrong parameter: %s\n", flag);
        exit(EXIT_FAILURE);
    }

}
