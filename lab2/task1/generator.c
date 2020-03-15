#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *generate_line(int length)
{
    if (length < 1)
    {
        return NULL;
    }

    char *line = malloc(length * sizeof(char) + 1);
    for (int i = 0; i < length; i++)
    {
        line[i] = (char) '0' + abs(rand()) % 76;
    }
    line[length] = '\n';

    return line;
}


int generate(char *fname, int line_num, int byte_num)
{
    FILE *file = fopen(fname, "w+");
    
    for (int i = 0; i < line_num; i++)
    {
        char *line = generate_line(byte_num);
        if (line == NULL)
        {
            return EXIT_FAILURE;
        }

        if (fwrite(line, sizeof(char), (size_t) byte_num + 1, file) != byte_num + 1)
        {
            return EXIT_FAILURE;
        }
    }

    fclose(file);

    return EXIT_SUCCESS;
}

void generate_check(char *fname, int line_num, int byte_num)
{
    if (generate(fname, line_num, byte_num) != 0)
    {
        printf("Error while generating file");
    }
}


int main(int argc, char *argv[])
{
    generate_check("file.txt", 200, 200);


    return 0;
}