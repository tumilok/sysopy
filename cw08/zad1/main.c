#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define SIGN 0
#define BLOCK 1
#define INTERLEAVED 2

#define MAX_LINE_LEN 2048
#define MAX_SCALE 256

int threads_num;
int exec_mode;
int **histogram;
int **image;
int width, height;

void error(char *msg)
{
	printf("%s Error: %s\n", msg, strerror(errno));
	exit(EXIT_FAILURE);
}

void save_to_file(char *fname)
{
	FILE *fp = fopen(fname, "w+");
	for (int i = 0; i < MAX_SCALE; i++)
	{
		fprintf(fp, "%d: %d\n", i, histogram[0][i]);
	}
}

void init_hist()
{
	histogram = calloc(threads_num, sizeof(int*));
	for (int i = 0; i < threads_num; i++)
	{
		histogram[i] = calloc(MAX_SCALE, sizeof(int));
	}

	for (int i = 0; i < threads_num; i++)
	{
		for (int j = 0; j < MAX_SCALE; j++)
		{
			histogram[i][j] = 0;
		}
	}
}

void free_arrays()
{
	for (int i = 0; i < height; i++)
	{
		free(image[i]);
	}
	free(image);

	for (int i = 0; i < threads_num; i++)
	{
		free(histogram[i]);
	}
	free(histogram);
}

void read_file(char *fname)
{
	FILE *fp = fopen(fname, "r");
    if (!fp)
	{
		error("Couldn't open file");
	}

	char buff[MAX_LINE_LEN];
	int line_idx = 0;
	int image_max_value;

    while(line_idx < 3 && fgets(buff, MAX_LINE_LEN, fp))
	{
        if (buff[0] == '#')
		{
			continue;
		}

        if (line_idx == 0 && strncmp("P2", buff, 2))
		{
            fclose(fp);
            error("Wrong image color mode. Expected [P2].");
        }
        else if(line_idx == 1)
		{
            if (sscanf(buff, "%d %d\n", &width, &height) != 2)
			{
                fclose(fp);
                error("Couldn't get width and height of image.");
            }

            image = (int **) calloc(height, sizeof(int *));
            for (int i = 0; i < height; i++)
			{
                image[i] = (int *) calloc(width, sizeof(int));
			}
        }
        else if(line_idx == 2)
        {
            if (sscanf(buff, "%d\n", &image_max_value) != 1)
			{
                fclose(fp);
                error("Couldn't get image max value.");
            }
        }
        line_idx++;
    }

    if (line_idx != 3)
	{
        fclose(fp);
        error("Couldn't parse image.");
    }

    int value;
    for (int row = 0; row < height; row ++)
	{
        for (int col = 0; col < width; col ++)
		{
            fscanf(fp, "%d", &value);
            image[row][col] = value;
        }
    }
    fclose(fp);
}

long time_diff(struct timeval start, struct timeval stop)
{
	return (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
}

void *create_hist(void *arg)
{
	struct timeval stop, start;

	int k = *((int*) arg);
	int x_start;
	int x_stop;

	gettimeofday(&start, NULL);

	switch (exec_mode)
	{
		case SIGN:
			for (int r = 0; r < height; r++)
			{
				for (int c = 0; c < width; c++)
				{
					if (image[r][c] % threads_num == k)
					{
						histogram[k][image[r][c]]++;
					}
				}
			}
			break;

		case BLOCK:
			x_start = k * ceil((double) width / threads_num);
			x_stop = (k + 1) * ceil((double) width / threads_num);
			for (int r = 0; r < height; r++)
			{
				for (int c = x_start; c < x_stop; c++)
				{
					histogram[k][image[r][c]]++;
				}
			}
			break;

		case INTERLEAVED:
			for (int r = 0; r < height; r++)
			{
				for(int c = k; c < width; c += threads_num)
				{
					histogram[k][image[r][c]]++;
				}
			}
			break;
	}

	gettimeofday(&stop, NULL);

	long* delta_us = malloc(sizeof(long));
	*(delta_us) = time_diff(start, stop);

	pthread_exit(delta_us);
}

pthread_t *init_threads()
{
	pthread_t *threads = calloc(threads_num, sizeof(pthread_t));
	for (int i = 0; i < threads_num; i++)
	{
		int *k = malloc(sizeof(int));
		*(k) = i;
		pthread_create(&threads[i], NULL, create_hist, k);
	}
	return threads;
}

int main(int argc, char *argv[])
{
	if (argc < 5)
	{
		error("Wrong number of arguments. Expected: threads_num sign/block/interleaved input_file output_file");
	}

	threads_num = atoi(argv[1]);
	if (threads_num <= 0)
	{
		error("Invalid threads number.");
	}

	if (!strcmp(argv[2], "sign"))
	{
		exec_mode = SIGN;
	}
	else if (!strcmp(argv[2], "block"))
	{
		exec_mode = BLOCK;
	}
	else if (!strcmp(argv[2], "interleaved"))
	{
		exec_mode = INTERLEAVED;
	}
	else
	{
		error("Invalid mode. Expected: sign/block/interleaved");
	}
	char *input_file = argv[3];
	char *output_file = argv[4];

	printf("Exec mode: %s\t Threads number: %s\t Input file: %s\t Output file: %s\n", argv[2], argv[1], argv[3], argv[4]);
	
	read_file(input_file);

	init_hist();

	struct timeval stop, start;

	gettimeofday(&start, NULL);

	pthread_t *threads = init_threads(threads_num);

	for (int i = 0; i < threads_num; i++)
	{
		void *val;
		if (pthread_join(threads[i], &val) > 1)
		{
			error("Couldn't finish thread");
		}
		long thread_time = *((long*) val);

		printf("Thread %d time: %ld ms\n", i+1, thread_time);
	}

	gettimeofday(&stop, NULL);

	printf("Full time: %ld ms\n\n", time_diff(start, stop));

	for (int i = 0; i < MAX_SCALE; i++)
	{
		for (int k = 1; k < threads_num; k++)
		{
			histogram[0][i] += histogram[k][i];
		}
	}
	
	save_to_file(output_file);

	free_arrays();

	exit(EXIT_SUCCESS);
}