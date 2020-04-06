#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>


#define PROD_NUM 5
#define PIPE "pipe"

#define PRODUCENT "./producer"
#define PROD_FNAME "files/file"
#define PROD_READ_SIZE "5"

#define CONSUMER "./consumer"
#define CONS_FNAME "files/result.txt"
#define CONS_READ_SIZE "10"

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

void generate_files()
{
    char fname[100];
    char number[2];

    for (int i = 0; i < PROD_NUM; i++)
    {
        char fname[100];
        sprintf(fname, "%s%d.txt", PROD_FNAME, i + 1);
        FILE *fp = fopen(fname, "w+");

        for (int j = 0; j < 10; j++)
        {
            char c = 'A' + (rand() % 26);
            fwrite(&c, sizeof(char), 1, fp);
        }
        fclose(fp);
    }
}

int main(int argc, char *argv[])
{
    if (mkfifo(PIPE, 0666) < 0)
    {
        error("could not create pipe");
    }

    generate_files();

    if (fork() == 0)
    {
        execl(CONSUMER, CONSUMER, PIPE, CONS_FNAME, CONS_READ_SIZE, NULL);
    }

    for (int i = 0; i < PROD_NUM; i++)
    {
        if (fork() == 0)
    {
        char fname[100];
        sprintf(fname, "%s%d.txt", PROD_FNAME, i + 1);
        execl(PRODUCENT, PRODUCENT, PIPE, fname, PROD_READ_SIZE, NULL);
    }
    }

    for (int i = 0; i < PROD_NUM + 1; i++)
    {
        wait(NULL);
    }

    return 0;
}