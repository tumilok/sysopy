#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 10
#define MAX_COMMANDS 10

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

void parse_command(char *args[], char *line)
{    
    int args_num = 0;
    char delims[2] = {' ', '\n'};
    char *arg = strtok(line, delims);

    while (arg != NULL)
    {
        args[args_num++] = arg;
        arg = strtok(NULL, delims);
    }

    if (args_num > MAX_ARGS)
    {
        error("number of arguments exceeded limit");
    }
}

int parse_line(char *commands[][MAX_ARGS], char *line)
{
    for (int i = 0; i < MAX_COMMANDS; i++)
    {
        for (int j = 0; j < MAX_ARGS; j++)
        {
            commands[i][j] = NULL;
        }
    }

    char *command = strtok_r(line, "|", &line);
    int commands_num = 0;
    while (command != NULL)
    {
        parse_command(commands[commands_num++], command);
        command = strtok_r(NULL, "|", &line); 
    }

    if (commands_num > MAX_COMMANDS)
    {
        error("number of comands exceeded limit");
    }
    return commands_num;
}

void exec_commands(char *line)
{
    char *commands[MAX_COMMANDS][MAX_ARGS];
    int commands_num = parse_line(commands, line);

    int pipes[commands_num - 1][2];
    for (int i = 0; i < commands_num - 1; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            error("cannot pipe");
        }
    }

    for (int i = 0; i < commands_num; i++)
    {    
        pid_t pid = fork();
        if (pid == 0)
        {
            if (i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i + 1 < commands_num)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < commands_num - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            execvp(commands[i][0], commands[i]);

            exit(EXIT_SUCCESS);
        }
        else if (pid < 0)
        {
            error("cannot fork");
        }
    }

    for (int i = 0; i < commands_num - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < commands_num; i++)
    {
        wait(0);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        error("wrong number of arguments");
    }

    char *fname = argv[1];
    FILE *fp = fopen(fname, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL)
    {
        error("cannot open file");
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        exec_commands(line);
    }

    fclose(fp);
    free(line);
    
    exit(EXIT_SUCCESS);
}