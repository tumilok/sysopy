#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/times.h>

clock_t start_time, end_time;
struct tms start_cpu, end_cpu;

void start_timer()
{
    start_time = times(&start_cpu);
}

void end_timer()
{
    end_time = times(&end_cpu);
}

void write_result(FILE* file, char* name)
{
    double real_t = (double) (end_time - start_time) / sysconf(_SC_CLK_TCK);
    double user_t = (double) (end_cpu.tms_utime - start_cpu.tms_utime) / sysconf(_SC_CLK_TCK);
    double system_t = (double)(end_cpu.tms_stime - start_cpu.tms_stime) / sysconf(_SC_CLK_TCK);
    
    fprintf(file, "Operation: %s\n", name);
    fprintf(file, "Real time: %f\n", real_t);
    fprintf(file, "User time: %f\n", user_t);
    fprintf(file, "System time: %f\n\n", system_t);
}


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


void generate(char *fname, int line_num, int byte_num)
{
    FILE *file = fopen(fname, "w+");
    
    for (int i = 0; i < line_num; i++)
    {
        char *line = generate_line(byte_num);
        if (line == NULL)
        {
            printf("ERROR");
            exit(1);
        }

        if (fwrite(line, sizeof(char), (size_t) byte_num + 1, file) != byte_num + 1)
        {
            printf("ERROR");
            exit(1);
        }
    }

    fclose(file);
}

// sorting

void swap_lines_sys(int fp, int len, int i, int j){
    char* a_line = calloc(len + 1, sizeof(char));
    char* b_line = calloc(len + 1, sizeof(char));

    lseek(fp, (len + 1) * i, 0);
    read(fp, a_line, len + 1);

    lseek(fp, (len + 1) * j, 0);
    read(fp, b_line, len + 1);

    lseek(fp, (len + 1) * j, 0);
    write(fp, a_line, len + 1);

    lseek(fp, (len + 1) * i, 0);
    write(fp, b_line, len + 1);

    free(a_line);
    free(b_line);
}

int partition_sys(int fp, int len, int low, int high)
{
    char* pivot = calloc(len + 1, sizeof(char));
    lseek(fp, (len + 1) * high, 0);
    read(fp, pivot, len + 1);
    
    int i = low - 1;
    char* line = calloc(len + 1, sizeof(char));
    for (int j = low; j < high; j ++)
    {
        lseek(fp, (len + 1) * j, 0);
        read(fp, line, len + 1);
        if (strcmp(line, pivot) < 0)
        {
            i++;
            swap_lines_sys(fp, len, i, j);
        }
    }
    swap_lines_sys(fp, len, i + 1, high);

    free(line);
    free(pivot);

    return i + 1;
}

void quick_sort_sys(int fp, int len,  int low, int high)
{
    if (low < high)
    {
        int pivot = partition_sys(fp, len, low, high);

        quick_sort_sys(fp, len, low, pivot - 1);
        quick_sort_sys(fp, len, pivot + 1, high);
    }
}

void swap_lines_lib(FILE* fp, int len, int i, int j){
    char* a_line = calloc(len + 1, sizeof(char));
    char* b_line = calloc(len + 1, sizeof(char));

    fseek(fp, (len + 1) * i, 0);
    fread(a_line, sizeof(char), len + 1, fp);

    fseek(fp, (len + 1) * j, 0);
    fread(b_line, sizeof(char), len + 1, fp);

    fseek(fp, (len + 1) * j, 0);
    fwrite(a_line, sizeof(char), len + 1, fp);

    fseek(fp, (len + 1) * i, 0);
    fwrite(b_line, sizeof(char), len + 1, fp);

    free(a_line);
    free(b_line);
}

int partition_lib(FILE* fp, int len, int low, int high)
{
    char* pivot = calloc(len + 1, sizeof(char));
    fseek(fp, (len + 1) * high, 0);
    fread(pivot, sizeof(char), len + 1, fp);
    
    int i = low - 1;
    char* line = calloc(len + 1, sizeof(char));
    for (int j = low; j < high; j ++)
    {
        fseek(fp, (len + 1) * j, 0);
        fread(line, sizeof(char), len + 1, fp);
        if (strcmp(line, pivot) < 0)
        {
            i++;
            swap_lines_lib(fp, len, i, j);
        }
    }
    swap_lines_lib(fp, len, i + 1, high);

    free(line);
    free(pivot);

    return i + 1;
}

void quick_sort_lib(FILE* fp, int len,  int low, int high)
{
    if (low < high)
    {
        int pivot = partition_lib(fp, len, low, high);

        quick_sort_lib(fp, len, low, pivot - 1);
        quick_sort_lib(fp, len, pivot + 1, high);
    }
}


void sort_file(char *fname, int line_num, int byte_num, char *lib) 
{
    if (strcmp(lib, "sys") == 0)
    {
        int fp = open(fname, O_RDWR);
        if (fp < 0)
        {
            printf("ERROR");
            exit(1);
        }
        quick_sort_sys(fp, byte_num, 0, line_num - 1);
        close(fp);
    }
    else if (strcmp(lib, "lib") == 0)
    {
        FILE* fp = fopen(fname, "r+");
        if (fp == NULL)
        {
            printf("ERROR");
            exit(1);
        }
        quick_sort_lib(fp, byte_num, 0, line_num - 1);
        fclose(fp);
    }
    else 
    {
        printf("ERROR");
        exit(1);
    }
}

// Copy

void copy_file_sys(int source_fp, int dest_fp, int line_num, int byte_num)
{
    char *line = calloc(byte_num + 1, sizeof(char));

    for (int i = 0; i < line_num; i++)
    {
        lseek(source_fp, (byte_num + 1) * i, 0);
        read(source_fp, line, byte_num + 1);
        lseek(dest_fp, (byte_num + 1) * i, 0);
        write(dest_fp, line, byte_num + 1);
    }
    free(line);
}

void copy_file_lib(FILE* source_fp, FILE* dest_fp, int line_num, int byte_num)
{
    char *line = calloc(byte_num + 1, sizeof(char));

    for (int i = 0; i < line_num; i++)
    {
        fseek(source_fp, (byte_num + 1) * i, 0);
        fread(line, sizeof(char), byte_num + 1, source_fp);
        fseek(dest_fp, (byte_num + 1) * i, 0);
        fwrite(line, sizeof(char), byte_num + 1, dest_fp);
    }
    free(line);
}

void copy_file(char *source, char *destination, int line_num, int byte_num, char *lib)
{
    if (strcmp(lib, "sys") == 0)
    {
        int source_fp = open(source, O_RDONLY);
        if (source_fp < 0)
        {
            printf("ERROR");
            exit(1);
        }

        int dest_fp = open(destination, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (dest_fp < 0)
        {
            printf("ERROR");
            exit(1);
        }

        copy_file_sys(source_fp, dest_fp, line_num, byte_num);

        close(source_fp);
        close(dest_fp);
    }
    else if (strcmp(lib, "lib") == 0)
    {
        FILE* source_fp = fopen(source, "r");
        if (source_fp == NULL)
        {
            printf("ERROR");
            exit(1);
        }

        FILE* dest_fp = fopen(destination, "w+");
        if (dest_fp == NULL)
        {
            printf("ERROR");
            exit(1);
        }
        
        copy_file_lib(source_fp, dest_fp, line_num, byte_num);

        fclose(source_fp);
        fclose(dest_fp);
    }
    else
    {
        printf("ERROR");
        exit(1);
    }
}

void read_arguments(int argc, char *argv[])
{
    char* fname = "wyniki.txt";
    FILE *rapport_fp = fopen(fname, "a");
    if (rapport_fp == NULL){
        exit(-1);
    }

    if (argc < 5)
    {
        printf("ERROR");
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "generate") == 0)
        {
            if (i + 3 >= argc)
            {
                printf("ERROR");
                exit(1);
            }
            generate(argv[i + 1], atoi(argv[i + 2]), atoi(argv[i + 3]));
            i+=3;
        }
        else if (strcmp(argv[i], "sort") == 0)
        {
            if (i + 4  >= argc)
            {
                printf("ERROR");
                exit(1);
            }
            char info[64];
            snprintf(info, sizeof info, "sort: %s, records: %s, bytes: %s", argv[i + 4], argv[i + 2], argv[i + 3]);
            start_timer();
            sort_file(argv[i + 1], atoi(argv[i + 2]), atoi(argv[i + 3]), argv[i + 4]);
            end_timer();
            write_result(rapport_fp, info);
            i+=4;
        }
        else if (strcmp(argv[i], "copy") == 0)
        {
            if (i + 5 >= argc)
            {
                printf("EROR");
                exit(1);
            }
            char info[64];
            snprintf(info, sizeof info, "copy: %s, records: %s, bytes: %s", argv[i + 5], argv[i + 3], argv[i + 4]);
            start_timer();
            copy_file(argv[i + 1], argv[i + 2], atoi(argv[i + 3]), atoi(argv[i + 4]), argv[i + 5]);
            end_timer();
            write_result(rapport_fp, info);
            i+=5;
        }
        else
        {
            printf("EROR");
            exit(1);
        }
    }
    
    fclose(rapport_fp);
}


int main(int argc, char *argv[])
{
    read_arguments(argc, argv);

    return 0;
}