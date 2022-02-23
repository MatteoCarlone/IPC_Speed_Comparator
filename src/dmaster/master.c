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

#define MODENUMBER 4

// Colors for konsole prints.

const char *red = "\033[0;31m";
const char *bhred = "\e[1;91m";
const char *green = "\033[1;32m";
const char *yellow = "\033[1;33m";
const char *cyan = "\033[0;36m";
const char *magenta = "\033[0;35m";
const char *bhgreen = "\e[1;92m";
const char *bhyellow = "\e[1;93m";
const char *bhblue = "\e[1;94m";
const char *bhmagenta = "\e[1;95m";
const char *bhcyan = "\e[1;96m";
const char *bhwhite = "\e[1;97m";
const char *reset = "\033[0m";

// Defining bool type.

typedef enum {
    false,
    true
}bool;

// Declaring two different booleans for the while loop.

bool mode = true;
bool cycle = true;

// Declaring Functions

int spawn(const char *pogram, char **arg_list);
void create_fifo(const char *name);
void mode_function(int arr[MODENUMBER]);
void replace(int arr[MODENUMBER], int x);

// Declaring time variables.

	time_t current_time;
	struct timeval tv={0,0};

// Function: CHECK(X).
// This function writes on the shell whenever a sistem call returns any kind of error.
// The function will print the name of the file and the line at which it found the error.
// It will end the check exiting with code 1.

#define CHECK(X) ({int __val = (X); (__val == -1 ? ({fprintf(stdout,"ERROR (" __FILE__ ":%d) -- %s\n",__LINE__,strerror(errno)); exit(-1);-1;}) : __val); })

int main(int argc, char const *argv[]){

	// Creating logfile.

	FILE *out = fopen("debug.txt", "w");
  fprintf(out, "################################## DEBUG FILE ################################## \n\n");
  fflush(out);
  CHECK(fclose(out));

	// Checking if the usage is clear.

	if (argc != 1){
    	fprintf(stderr, "Usage:%s <filename>\n", argv[0]);
    	exit(-1);
	}

	// Declaring int variables, circular buffer sizes for mode 4,
	// buffer size for the size of the data and input mode for 
	// the input mode choose.

	int circular_buffer_size = 0;
	int buffer_size;
	int input_mode = 10;

	// Declaring char variables.

	char circular_buffer_size_s[80];
	char buffer_size_s[80];

	pid_t pid_cb;

	// Declaring double time variables got from either processes,
	// producer and consumer.

	double time_p, time_c;

	// Opening again the file to append infos.

	FILE *out2 = fopen("debug.txt", "a");

	// Creating fifos to comunicate with producer and consumer.

  create_fifo("/tmp/my_time_p");
  create_fifo("/tmp/my_time_c");

  // Declaring file descriptors for the pipes with the consumer
  // or the producer.

	int fd_producer, fd_consumer;

	// Declaring the array containing the numbers of the modes, 
	// if the user will decide to keep using one buffer size to 
	// compare them, the replace() function will change some 
	// values to 0 to don't let the user re-test them.

	int modes_array[MODENUMBER] = {1,2,3,4};

	// File descriptor of the unnamed pipe

  int fd_up[2];

  // File descriptor of the named pipe.

  char *named_pipe = "/tmp/named_pipe";

  // Declaring value string, it'll be used for the buffer.

	char value[256];

	// Printing PID of the master.

  printf("\nPID master [%d]\n", getpid()); fflush(stdout);

  bool YesNo_b;

  // We're looping to keep the questions ready.

	while(1){

		// We're using a switch to change the ask in two situations.
		// First is the case where the user wants to change the buffer
		// size, the second one is to ask the user if he wants to keep
		// the buffer size. In both cases there's the opportunity to
		// quit the program.

		char YesNo_s[256];

		switch(mode){

			case true:

				// We keep asking the user for the input if it is not right.

				while(buffer_size > 100 || buffer_size <0 || !buffer_size){

  				printf("\n%s Insert buffer size [MB] smaller than 100MB!\n You can quit by typing [quit]!%s \n\n", bhblue, reset);
  				scanf("%s", value);
  				buffer_size = atoi(value);
  				if (buffer_size > 100 || buffer_size < 0 || !buffer_size || buffer_size == 0){
  						if (!strcmp(value, "quit")){
  							CHECK(fclose(out2));
  							return 0;
  						}
        			printf("\n%s Wrong value! try again:%s\n\n", bhred, reset); fflush(stdout);
     				}
   			}

   			fprintf(out2, "New buffer size: %d MBs", buffer_size);
   			time(&current_time);
    		fprintf(out2, "                                                 Time:  %s", ctime(&current_time));

     		cycle = true; // We turn cycle true, to let the input mode start.

     		// As we're sending int datas, we know that for each megabyte we need
     		// a size of 25000.

   			buffer_size = buffer_size*250000;

   			sprintf(buffer_size_s, "%d", buffer_size); // Passing buffer_size in a string.

			break;

			case false:
				
				// We are asking to the user if he either wants to maintain the buffer size or going for another one.
				// This while is really important for the user experience becayse it manages if the user wants to go
				// for a new buffer size or mantain the old one.

				YesNo_b = false;

				printf("\n Would you like to try the same buffer size with another mode? [Y/n]\n To quit type [quit]!\n\n");

				while(YesNo_b == false){

					scanf("%s", YesNo_s);

					// If the user says yes we mantain the buffer size.

					if(!strcmp(YesNo_s, "y") || !strcmp(YesNo_s, "Y")){

						printf("\n You decided to mantain the same buffer size.\n");
						fprintf(out2, "User decided to mantain the same buffer size: %d MBs", buffer_size/250000);
						time(&current_time);
    				fprintf(out2, "                    Time:  %s", ctime(&current_time));
						cycle = true; // We restart the modes cycle.
						YesNo_b = true; // We exit the asking loop.

						// If all the input modes has been used, we restart the asking while of the buffer size.

						if(modes_array[0] == 0 && modes_array[1] == 0 && modes_array[2] == 0 && modes_array[3] == 0){
							printf(" You have just tried all the modes with this buffer size, write a new one!\n");
							mode = true; // We restart the asking start.
					 		cycle = false; // We stop the input mode cycle.
						}
					}

					// If he says no we modify it.

					else if(!strcmp(YesNo_s,"n") || !strcmp(YesNo_s,"N")){

					 	printf("\n You decided not to mantain the same buffer size.\n");
					 	fprintf(out2, "The user decided not to mantain the same buffer size.");
					 	time(&current_time);
    				fprintf(out2, "                   Time:  %s", ctime(&current_time));

					 	// We "refresh" the modes array, we want the user able to choose
					 	// again whatever input mode he wants.

					 	for(int i=0;i<MODENUMBER; i++) modes_array[i] = i+1;

					 	mode = true; // We restart the asking start.
					 	cycle = false; // We stop the input mode cycle.
					 	YesNo_b = true; // We exit the while.
					}

					// If the user presses 'q' we quit.

					else if(!strcmp(YesNo_s,"quit")) {					
  					CHECK(fclose(out2));
						return 0;
					}

					// If the user presses something else, we ask him to type again.

					else printf("\n%s Wrong value! try again.%s\n\n", bhred, reset); fflush(stdout);
				}

			break;
		}

		while(cycle){

			// Declaring the input_mode variable string.

			char input_mode_s[256];

			// Asking the user which mode of data sharing would he like to use.

			printf("\n You can quit by typing [quit]!\n Which data sharing mode do you want to use? Press: \n");

			// Printing the data sharing modes.

			mode_function(modes_array);

			// If the input_mode is not a right key to press we keep asking the user for a new input.
			// We use a while to manage bad inputs.

			while(input_mode != 1 && input_mode != 2 && input_mode != 3 && input_mode != 4){

				// Reading the input.

				scanf("%s", input_mode_s);
				input_mode = atoi(input_mode_s); // Turning it into an int.

				// Checking if there's any input repeated within the possibility for the user 
				// to use the same buffer size for comparison (see the YesNo while() loop up
				// in the code).

				for(int i=0; i<4; i++){
					if(input_mode == i+1 && modes_array[i] == 0){
					printf("%s\n You have already tried this mode, try a new one!%s\n\n", bhred, reset); fflush(stdout);
					input_mode = 999; // Random value.
					}
				}

				// if the input is wrong we let the user know it.

				if ((input_mode != 1 && input_mode != 2 && input_mode != 3 && input_mode != 4) || input_mode == 0){

					if(!strcmp(input_mode_s, "quit")) {
  					CHECK(fclose(out2));
						return 0; // If the user wants to quit the program, he can.
					}
					if(input_mode != 999) {
						printf("%s\n Wrong key, try again!%s\n", bhred, reset); fflush(stdout);
					}

				}
				else mode = false; // To let the user choose how to continue the program, we turn the mode false.
			}

			// Now we get in the core of the code, here we can see the the different data sharing modes 
			// that are manage through a variable called input_mode, the one we got through the scanf()
			// above on the code. We can have four different data sharing modes, that we will call with
			// the spawn() function which is implemented at the bottom of the code. the four modes are:
			// 1. Unnamed pipes.
			// 2. Named pipes.
			// 3. Socket.
			// 4. Circular buffer with data shared.

			switch(input_mode){

				// UNNAMED PIPES MODE.

				case 1:

					printf("\n%s [1]: Un-named pipes%s\n", bhyellow, reset);
    			printf("%s -----------------------%s\n", bhyellow, reset);
    			printf(" Sending data...\n"); fflush(stdout);

    			// Saving the current time.

        	time(&current_time);
    			fprintf(out2, "User decided to use modality 1, unnamed pipe.");
    			fprintf(out2, "                           Time:  %s", ctime(&current_time));

    			// Declaring file descriptors.

    			char fd_1[80];
    			char fd_2[80];

					CHECK(pipe(fd_up) == -1);

					// Assigning the two variables for reading and writing 
					// the unnamed pipe.

					sprintf(fd_1, "%d", fd_up[1]);
					sprintf(fd_2, "%d", fd_up[0]);

					char *arg_list_up[] = {"./up", fd_1, fd_2, buffer_size_s, NULL};

					// Spawning of the process.

					pid_t pid_up = spawn("./up", arg_list_up);

					// Calling replace() function to change the array.

					replace(modes_array,1);

					// Stopping the cycle.

					cycle = false;

					printf(" PID UP [%d]\n", pid_up); fflush(stdout);

					// Turning input_mode to 5 in order to get inside the while()
					// loop to ask the input_mode again.

					input_mode = 5;

				break;

				// NAMED PIPES MODE.

				case 2:

					printf("\n%s [2]: Named pipes:%s\n", bhgreen, reset);
					printf("\n%s -----------------------%s\n", bhgreen, reset);
					printf(" Sending data...\n"); fflush(stdout);

					fprintf(out2, "User decided to use modality 2, named pipe.");
					time(&current_time);
    			fprintf(out2, "                             Time:  %s", ctime(&current_time));

					// Creating pipes.

					create_fifo(named_pipe);

					// Creating argument list.

					char *arg_list_np[] = {"./np", named_pipe, buffer_size_s, NULL};

					// Spawning of the process.

					pid_t pid_np = spawn("./np", arg_list_np);

					// Calling replace() function to change the array.

					replace(modes_array,2);

					// Stopping the cycle.

					cycle = false;

					printf(" PID Named Pipe [%d]\n", pid_np); fflush(stdout);

					// Turning input_mode to 5 in order to get inside the while()
					// loop to ask the input_mode again.

					input_mode = 5;

				break;

				// SOCKETS MODE.

				case 3:

					printf("\n%s [3]: Sockets:%s\n", bhmagenta, reset);
					printf("\n%s -----------------------%s\n", bhmagenta, reset);
					printf(" Sending data...\n"); fflush(stdout);

					fprintf(out2, "User decided to use modality 3, socket.");
					time(&current_time);
    			fprintf(out2, "                                 Time:  %s", ctime(&current_time));

					// Initialization of the array of data needed for using the execvp() function
					// contained in the spawn() function.

					char *arg_list_socket[] = {"./socket", "5096", "127.0.0.1", buffer_size_s, (char *)NULL};

					// Spawning the process.

					pid_t pid_socket = spawn("./socket", arg_list_socket);

					// Calling replace() function to change the array.

					replace(modes_array,3);

					// Stopping the cycle.

					cycle = false;

	  			printf(" PID socket [%d]\n", pid_socket); fflush(stdout);

	  			// Turning input_mode to 5 in order to get inside the while()
					// loop to ask the input_mode again.

	  			input_mode = 5;

				break;

				// CIRCULAR BUFFER WITH SHARED MEMORY MODE.

				case 4:

					fprintf(out2, "User decided to use modality 4, circular buffer with shared memory.");
					time(&current_time);
    			fprintf(out2, "     Time:  %s", ctime(&current_time));

					// Asking the user how big he wants the circular buffer memory.

					printf("\n How big should the circular-buffer memory be [KB]? (0.1-10): \n");

					do{

						scanf("%d", &circular_buffer_size);

						if (circular_buffer_size <= 0 || circular_buffer_size > 10){
							printf("%s Wrong mem! try again:%s\n", bhred, reset);
							fflush(stdout);
						}

	    		} while (circular_buffer_size <= 0 || circular_buffer_size > 10);

	    		fprintf(out2, "User decide to use %d KBs as circular buffer size", circular_buffer_size);
   				time(&current_time);
    			fprintf(out2, "                       Time:  %s", ctime(&current_time));

					circular_buffer_size = circular_buffer_size*250;

					sprintf(circular_buffer_size_s, "%d", circular_buffer_size);

					printf("\n%s [4]: Shared memory with circular buffer:%s\n", bhcyan, reset);
					printf("\n%s -----------------------%s\n", bhcyan, reset);
					printf(" Sending data...\n"); fflush(stdout);

					// Initialization of the array of data needed for using the execvp() function
					// contained in the spawn() function.

					char *arg_list_1[] = {"./cb", buffer_size_s, circular_buffer_size_s, (char *)NULL};

					// Spawning the process.

					pid_cb = spawn("./cb", arg_list_1);

					// Calling replace() function to change the array.

					replace(modes_array,4);

					// Stopping the cycle.

					cycle = false;

					printf(" PID Circular-buffer [%d]\n", pid_cb); fflush(stdout);

					// Turning input_mode to 5 in order to get inside the while()
					// loop to ask the input_mode again.

					input_mode = 5;

				break;
					
		}
		
		// Opening FIFOs to read time from the processes.

		CHECK(fd_producer = open("/tmp/my_time_p", O_RDONLY));
		CHECK(fd_consumer = open("/tmp/my_time_c", O_RDONLY));
		sleep(1);
		
		// Waiting and getting the status of the closing process.

		int status;
		CHECK(wait(&status));

		if (WIFEXITED(status))
		{
		  CHECK(WEXITSTATUS(status));
		}

		// Reading both times of producer and consumer.

		read(fd_producer, &time_p, sizeof(double));
		read(fd_consumer, &time_c, sizeof(double));

		// Calculating the time difference.

		double timediff = time_c - time_p;

		// Printing times and time diff.

		printf("\n TIME 1 IS: %lf\n", time_p/1000000000); fflush(stdout);
		printf(" TIME 2 IS: %lf\n", time_c/1000000000); fflush(stdout);
		printf(" TIME DIFFERENCE IS: %lf\n", timediff/1000000000); fflush(stdout);
		fprintf(out2, "Time difference is: %lf s", timediff/1000000000);
		time(&current_time);
   	fprintf(out2, "                                          Time:  %s", ctime(&current_time));



		// Really important switch, this switch case permits to close file
		// descriptors or unlinking pipes, in order to have a cleaner wor-
		// kspace.

		switch (input_mode){

	    case 1:
	      close(fd_up[0]);
	      close(fd_up[1]);
	      printf("%s -----------------------%s\n", bhyellow, reset); fflush(stdout);
	    break;

	    case 2:
	      unlink(named_pipe);
	      printf("%s -----------------------%s\n", bhgreen, reset); fflush(stdout);
	    break;

	    case 3:
	      printf("%s -----------------------%s\n", bhmagenta, reset); fflush(stdout);
	    break;

	    case 4:
	      printf("%s -----------------------%s\n", bhcyan, reset); fflush(stdout);
	    break;
	    }
		}
	}

	// Anyway, we want to unlink the FIFOs for time producer and consumer.

	unlink("/tmp/my_time_p");
	unlink("/tmp/my_time_c");
  CHECK(fclose(out2));
	return 0;
}



// Function: replace(__,__).
// The function replaces a 0 within the array of the sending modes.

void replace(int arr[MODENUMBER],int x){

	for(int i = 0 ; i < MODENUMBER ; i++){
		if(arr[i]==x){
			arr[i]=0;
		}
	}
}



// Function: mode_function(__).
// The function asks for the user the sending mode of the data.

void mode_function(int arr[MODENUMBER]){

	for(int i=0 ; i<MODENUMBER ; i++){
		switch(arr[i]){
			case 1:
				printf("\n [1]: Un-named pipes\n");
			break;
			case 2: 
				printf("\n [2]: Named pipes\n");
			break;
			case 3:
				printf("\n [3]: Sockets\n");
			break;
			case 4:
				printf("\n [4]: Shared Memory with Circular Buffer\n");
			break;

			case 0:

				printf("\n ---------------- \n");

			break;
		}
	}
	printf("\n");
}



// Function: create_fifo(__)
// The function creates a named pipe by passing the name of the file.

void create_fifo(const char *name){

  // automatically checks for errors.
  if (mkfifo(name, 0666) == -1){

    if (errno != EEXIST){

      perror("Error creating named fifo\n");
      exit(1);
    }
  }
}



// Function: spawn(__,__)
// This function forks the program and, on the child process, calls the function execvp()
// with the parameters passed as arguments of spawn().

int spawn(const char *program, char **arg_list){

  pid_t child_pid = fork();
  CHECK(child_pid);

  if (child_pid != 0) {return child_pid;}

  else{
    execvp(program, arg_list);
    // Check for errors of execvp.
    perror("exec failed");
    return 1;
  }
}
