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

// Defining a size to divide the array of data 
// that we want to send. This part was necessary 
// because an array can contain a maximum of 8 Gb 
// and we want to send datas from 0.1 to 100 Gb.

#define SIZE 200000

// Declaring Functions.

void bar(int percent_done, int bufSize);

void fillBuffer(int buf[], int bufSize);

int main(int argc, char *argv[])
{	

	// Declaring the variables needed to send datas 
	// and to get the time starting time and the finishing time.

	struct timespec start, finish;
	double time;
	int buffer[SIZE];
	int bufSize = atoi(argv[2]);

	// Producer Process (Child).

	if(fork() == 0){

		// Filling the buffer using the function fillBuffer
		// if few datas are sent we don't need to divide the array.

		if (bufSize < SIZE) fillBuffer(buffer, bufSize);
		else fillBuffer(buffer, SIZE);

		// Opening the named fifo in Write-Only mode.
		
		int fd = open(argv[1], O_WRONLY);

		// Getting the starting time of the process.

		clock_gettime(CLOCK_REALTIME, &start);

		// Writing the datas in packages of 200000 characters 
		for (int i = 0; i < bufSize - 1; i++){
	    	write(fd, &buffer[(i) % SIZE], sizeof(int));
		}

		// Converting the starting time in a struct of 
		// seconds and nanoseconds
		time = 1000000000 * (start.tv_sec) + (start.tv_nsec);

		// Opening the fifo to send the starting time to 
		// the Master Process

		int fd_fifo = open("/tmp/my_time_p", O_WRONLY);

		// Writing the the starting time on the fifo
		// that we have just opened.

		write(fd_fifo, &time, sizeof(double));

		// Closing the file descriptors

		close(fd_fifo);

		close(fd);
		
		return 0;
	}

	// Consumer Process (Parent).

	else{

		// Opening the named fifo in Reading-Only mode.
		int fd = open(argv[1], O_RDONLY);

		// Reading the datas in packages of 200000 characters
		for (int i = 0; i < bufSize - 1; i++){

			read(fd, &buffer[(i) % SIZE], sizeof(int));

			// Adding a loading bar to understand the 
			// duration of the process.

			if(i % (bufSize/100) == 0){
				bar(i,bufSize);
			}
		}

		// Getting the finishing time 

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

		// Closing file descriptors

		close(fd_fifo);

		close(fd);
		
		return 0;
	}
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