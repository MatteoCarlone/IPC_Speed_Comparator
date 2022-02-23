// Including Libraries

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>

// Defining a size to divide the array of data 
// that we want to send. This part was necessary 
// because an array can contain a maximum of 8 Gb 
// and we want to send datas from 0.1 to 100 Gb.

#define SIZE 200000

// Function: CHECK(X)
// This function writes on the shell whenever a sistem call returns any kind of error.
// The function will print the name of the file and the line at which it found the error.
// It will end the check exiting with code 1.

#define CHECK(X) (                                                 \
	{                                                              \
		int __val = (X);                                           \
		(__val == -1 ? (                                           \
						   {                                       \
							   fprintf(stderr, "ERROR ("__FILE__   \
											   ":%d) -- %s\n",     \
									   __LINE__, strerror(errno)); \
							   exit(EXIT_FAILURE);                 \
							   -1;                                 \
						   })                                      \
					 : __val);                                     \
	})

// Declaring Functions

void bar(int percent_done, int bufSize);

void fillBuffer(int buf[], int bufSize);

int main(int argc, char *argv[])
{
	// Declaring the buffer size chosen by the user.
	int buffer_size = atoi(argv[1]);
	// Declaring the size of the circular buffer chosen by the user.
	int circular_buffer_size = atoi(argv[2]);

	// Initializing, Opening and Creating named semaphores a the mutex
	sem_t *not_empty = sem_open("/sem_1", O_CREAT, S_IRUSR | S_IWUSR, 0);
	sem_t *not_full = sem_open("/sem_2", O_CREAT, S_IRUSR | S_IWUSR, buffer_size - 1);
	sem_t *mutex = sem_open("/sem_3", O_CREAT, S_IRUSR | S_IWUSR, 1);

	/*
		mmap() creates a new mapping in the virtual address space of the
   		calling process.
   		arguments:
   			* addres: set to NULL, the kernel will choose the address to the create the mapping
   			* the lenght of the mapping accordinding to which datas will be shared
   			* The Read-Write Protocol
   			* To argument to specify that the memory will be shared and the second one 
   			  is not backed by any file; its contents are initialized to zero. The fourth argument, 
   			  the fd is ignored and set to -1 and the last one (offset) is set to zero.
	*/ 
	int *buffer = mmap(NULL, circular_buffer_size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	// checking if the mapping is succesfull or not 
	if (buffer == MAP_FAILED)
	{
		printf("Mapping Failed\n");
		fflush(stdout);
		return -1;
	}

	// Child Producer.

	if (fork() == 0)
	{	

		// Declaring variables, time structures and
		// the array to send datas.
		// if few datas are sent we don't need to divide the array.
		int in = 0;
		double time;
		struct timespec start;
		int producer_arr[SIZE];

		if (buffer_size < SIZE) fillBuffer(producer_arr, buffer_size);
		else fillBuffer(producer_arr, SIZE);

		// Getting the startinf time of the process
		clock_gettime(CLOCK_REALTIME, &start);

		// Converting the starting time in a struct of
    	// seconds and nanoseconds

		time = 1000000000 * (start.tv_sec) + (start.tv_nsec);

		/*	
			Writing Datas on the circular buffer protecting the access
			to the shared memory using: 
				* mutex for the single access
				* semaphores to handle the quantity of data written in the circular buffer that can not 
				  exceed the max size.
		*/
		for(int i=0 ; i < buffer_size; i++)
		{
			sem_wait(not_full);
			sem_wait(mutex);
			buffer[in] = producer_arr[i % SIZE];
			in = (in + 1) % circular_buffer_size;
			sem_post(mutex);
			sem_post(not_empty);
		}

		// Opening the fifo to send the starting time
    	// to the Master Process.
		int fd_fifo = open("/tmp/my_time_p", O_WRONLY);

		// Writiing the starting time on the fifo 
    	// that we have just opened .
		write(fd_fifo, &time, sizeof(double));

		close(fd_fifo);

		return 0;
	}

	// Parent consumer.

	else
	{	
		// Declaring variables, time structures and
		// the array to receive datas.
	
		int out = 0;
		double time;
		int receiver_arr[SIZE];
		struct timespec finish;

		/*
			Writing Datas on the circular buffer protecting the access
			to the shared memory using: 
				* mutex for the single access
				* semaphores to handle the quantity of data written in the circular buffer that can not 
				  exceed the max size.
		*/

		for(int i=0 ; i < buffer_size ; i++)
		{
			sem_wait(not_empty);
			sem_wait(mutex);
			receiver_arr[i % SIZE] = buffer[out];
			fflush(stdout);
			out = (out + 1) % circular_buffer_size;
			sem_post(mutex);
			sem_post(not_full);

			// Adding a loading bar to understand the 
            // duration of the process.

			if(i % (buffer_size/100) == 0){
				bar(i,buffer_size);
			}
			
		}

		// Getting the finishing time of the process.

		clock_gettime(CLOCK_REALTIME, &finish);

		// Converting the finishing time in a struct of
        // seconds and nanoseconds

		time = 1000000000 * (finish.tv_sec) + (finish.tv_nsec);

		// Opening the fifo to send the finishing time
        // to the Master Process.

		int fd_fifo = open("/tmp/my_time_c", O_WRONLY);

		// Writiing the finishing time on the fifo 
        // that we have just opened .

		write(fd_fifo, &time, sizeof(double));

		close(fd_fifo);

		return 0;
	}

	// Deleting the shared memory block
	int err;
	CHECK(err = munmap(buffer, buffer_size));

	return 0;
}


void bar(int percent_done,int bufSize){

	/*
        Function to print a loading bar rescaled 
        to be 30 characthers, as arguments:

            * percent_done : the increase of the process 

            * bufSize : the size of the array datas

    */


    const int PROG_BAR_LENGHT = 30;

    int num_chars = (percent_done/(bufSize/100)) * PROG_BAR_LENGHT / 100 ;

    printf("\r[");
    for(int i = 0 ; i <= num_chars ; i++){
        printf("#");
    }
    for(int i = 0 ; i < PROG_BAR_LENGHT - num_chars-1; i++){
        printf(" ");
    }
    printf("] %d%% DONE", percent_done/(bufSize/100)+1);
    fflush(stdout);
}

void fillBuffer(int buf[], int bufSize)
{
	/*
        Function to fill the buffer of random integers
        According to the size chosen by the User.
    */
	for (int i = 0; i < bufSize; i++)
	{
		buf[i] = rand() % 9;
	}
}
