#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

/*
    I decided to pick SIGSEGV, SIGCLD and SIGFPE as signals to test
    SA_SIGINFO flag in three different scenarios:
        1, Segmentation fault caused by writing into read-only memory
        2. Child process finished the work
        3. Floating-point exception caused by division of integer by zero
*/

void segfault_handler(int sig_no, siginfo_t *info, void *ucontext)
{
    printf("Segmentation fault\n");   
    printf("Address of faulting memory reference: %p\n", info -> si_addr);
    
    exit(EXIT_SUCCESS);
}

void child_handler(int sig_no, siginfo_t *info, void *ucontext)
{
    printf("Child process has finished with status: %d\n", info -> si_status);
    printf("Child process PID: %d\n", info -> si_pid);
    printf("Real user UID of process than sent the signal: %d\n", info -> si_uid);
}

void floatex_handler(int sig_no, siginfo_t *info, void *ucontext)
{
    printf("Floating point exception\n");
    if (info -> si_code == FPE_INTDIV)
    {
        printf("Integer divide-by-zero\n");
    }
    else if (info -> si_code == FPE_FLTDIV)
    {
        printf("Floating point divide-by-zero\n");
    }
    else
    {
        printf("Unknow error\n");
    }
    exit(EXIT_SUCCESS);
}

void do_segfault()
{
    printf("\nProvoking segmentational fault\n");
    char *str = "Hello World";
    str[20] = 'z';
}

void do_child()
{
    pid_t forked = fork();
    if (forked == -1)
    {
        perror("sigtest: fork");
        exit(0);
    }
    else if (forked == 0)
    {
        printf("\nIt's me, your child!\n");
        exit(rand());
    }
    else
    {
        pause();
    }
}

void do_floatex()
{
    printf("\nProvoking error by dividing integer by zero\n");
    int num = rand();
    int zero = 0;
    num /= zero;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error, wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if (strcmp(argv[1], "segfault") == 0)
    {
        act.sa_sigaction = segfault_handler;
        sigaction(SIGSEGV, &act, NULL);
        do_segfault();
    }    
    else if (strcmp(argv[1], "child") == 0)
    {
        act.sa_sigaction = child_handler;
        sigaction(SIGCLD, &act, NULL);
        do_child();
    }
    else if (strcmp(argv[1], "floatex") == 0)
    {
        act.sa_sigaction = floatex_handler;
        sigaction(SIGFPE, &act, NULL);
        do_floatex();
    }
    else
    {
        printf("Error, wrong argument: %s\n", argv[1]);
    }

    return 0;
}