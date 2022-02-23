// Including Libraries

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

// Defining a size to divide the array of data 
// that we want to send. This part was necessary 
// because an array can contain a maximum of 8 Gb 
// and we want to send datas from 0.1 to 100 Gb.

#define SIZE 200000

// Function: CHECK(X).
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

void dostuff(int sock, int bufSize);

void fillBuffer(int buf[], int bufSize);

void bar(int percent_done, int bufSize);

void error(char *msg);


int main(int argc, char *argv[])
{       
    /*
        Declaring Variables:

            * sockfd and newsockfd: file descriptor returned by the socket system call
            * portno: stores the port number on which the server accepts connections
            * clilen: stores the size of the address of the client. This is needed for the accept system call.
            * n: is the return value for the read() and write() calls; 
                 i.e. it contains the number of characters read or written.
            * sockaddr_in serv_addr, cli_addr:
                * sockaddr_in: structure containing an internet address. This structure is defined in <netinet/in.h>
                * serv_addr: contain the address of the server
                * cli_addr: will contain the address of the client which connects to the server.
            * The buffers into which the server stores the charachters read from the socket connection.
            * pid: to store the process id of this program.
            * option: integer set at 1 needed int setsockopt function
    */
    
    int sockfd, newsockfd;
    int portno = atoi(argv[1]);
    socklen_t clilen;
    int n;
    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *server;
    int bufSize = atoi(argv[3]);
    int buffer[SIZE];
    int pid;
    int option = 1;

    // Server Process (Child).

    if(fork() == 0){
        /*
         creating a new socket using the function socket():
            Arguments:
                * AF_INET: to specify the internet domain of the address
                * SOCK_STREAM: the type of the socket, in this case the charachters will be read in a continuous stream
                * The protocol, if zero the operating system will choose the most appropriate protocol. 
                  In this case TCP for stream sockets
             Returns:
                * sockfd: file descriptor
        */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        // the file descriptor should not be less than zero.
        if (sockfd < 0) error("ERROR opening socket");

        // Funtion introduced to reuse the port number on which the server accept the connection with the client.
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        //initializing serv_addr to zeros.
        bzero((char *)&serv_addr, sizeof(serv_addr));

        /*
         Setting fields of the struct serv_addr:
            * sin_family: contains a code for the address family 
            * sin_addr.s_addr: This field contains the IP address of the host. 
                               For server code, this will always be the IP address of the machine on which the server
                               is running. To get we use the symbolic constant INADDR_ANY.
            *sin_port: contain the port number, htons() convert the port number in host byte order to a port number
                       in network byte order.

        */
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        /*
            The bind() system call binds a socket to an address, 
            in this case the address of the current host and port number on which the server will run
        */
        if (bind(sockfd, (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr)) < 0)
            error("ERROR on binding");

        // The listen system call allows the process to listen on the socket for connections
        listen(sockfd, 5);

        //The accept() system call causes the process to block until a client connects to the server
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,
                           (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        // Calling the function dostuff() explained later in the code.
        // This function mainly handle the comunication once a connection between client and server has been established.
        dostuff(newsockfd, atoi(argv[3]));

        // closing the file descriptors
        close(newsockfd);
        close(sockfd);
        
        return 0;
    }

    // Client Process (Parent).

    else{

        // explained in the server process
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            error("ERROR opening socket");
        // Funtion that, given the name of a host on the internet, 
        // returns a pointer to a struct containig information about the host, for instance its address.
        server = gethostbyname(argv[2]);
        if (server == NULL)
        {
            fprintf(stderr, "ERROR, no such host\n");
            exit(0);
        }


        /*
             This code sets the fields in serv_addr. Much of it is the same as in the server.
             the field server->h_addr is a character string, we use the function:
                      void bcopy(char *s1, char *s2, int length) 
             which copies length bytes from s1 to s2.
        */
        ///////////////////////
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(portno);
        ///////////////////////

        // The connect function is called by the client to establish a connection to the server.

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR connecting");
            
        // Initialing the buffer of zeros
        bzero(buffer, SIZE);
        
        // Declaring time variables and structs.
        double time;
        struct timespec finish;

        // Reading datas once the connection has been established
        for (int i = 0; i < bufSize; i++)
        {
            CHECK(read(sockfd, &buffer[(i) % SIZE], sizeof(int)));

            // Adding a loading bar to understand the 
            // duration of the process.

            if(i % (bufSize/100) == 0){
                bar(i,bufSize);
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

        close(sockfd);

        return 0;
    }
}


/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/

void dostuff(int sock, int bufSize)
{
    // Declaring time variables and structs.
    double time;
    struct timespec start;

    // Declaring, Initializing and filling the buffer.
    // if few datas are sent we don't need to divide the array.
    int buffer[SIZE];
    bzero(buffer, SIZE);
    
    if (bufSize<SIZE) fillBuffer(buffer, bufSize);
    else fillBuffer(buffer, SIZE);

    // Getting the starting time of the process.
    clock_gettime(CLOCK_REALTIME, &start);

    // Converting the starting time in a struct of
    // seconds and nanoseconds

    time = 1000000000 * (start.tv_sec) + (start.tv_nsec);

    // Writing datas one the connection has been established.

    for (int i = 0; i < bufSize; i++)
    {
        CHECK(write(sock, &buffer[(i) % SIZE], sizeof(int)));
    }

    // Opening the fifo to send the starting time
    // to the Master Process.
    int fd_fifo = open("/tmp/my_time_p", O_WRONLY);

    // Writiing the starting time on the fifo 
    // that we have just opened .
    write(fd_fifo, &time, sizeof(double));
    
    close(fd_fifo);
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

void error(char *msg)
{
    perror(msg);
    exit(0);
}
