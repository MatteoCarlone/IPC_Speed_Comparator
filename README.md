IPC_Speed_Comparator
--------------------

      ____  _____ _____ _____     _____ _____ _____ ____  _____ _____ 
     |    \|  _  |_   _|  _  |___|   __|   __|   | |    \|   __| __  |
     |  |  |     | | | |     |___|__   |   __| | | |  |  |   __|    -|
     |____/|__|__| |_| |__|__|   |_____|_____|_|___|____/|_____|__|__|

Welcome to the Data-Sender, created by Matteo Carlone and Luca
Predieri (ML99).
To install the program run:

$ source ./install.sh <pathname>

To run the program, run the command:

$ ./run.sh

To get info about the program run (NOTE: run it after you've inst
lledit):

$ ./help.sh

The data sender is a program created for comparing different kin-
ds of modalities to send datas between processes. As you can see 
there are 4 modalities:

1. Named pipes.
2. Unnamed pipes.
3. Socket.
4. Shared memory with circular buffer.

To let the user choose the modality and different important deci-
sions, we decided to create a simple user interface with differe-
nt parts. I will explain briefly what the user is asked to say:

1. The size of the data that the user wants to send.
2. The modality with which the user wants to send data.
   (if the modality chosen is the circular buffer, we need to ask
   the user which is the size that he wants to use for the circu-
   lar buffer).
3. If the user wants to keep that amount of data or change with a
   a new size. 
   
The user interface helps the user to travel inside the program.
All the questions can be easily dropped out by typing 'quit' on 
the console.
The data that travels cannot be decided by the user, it is just a 
simple buffer of ints with an amount as big as the Mbs decided by
the user at the beginning of the program, then it is all automat-
ized with the fillbuffer() function that is alive in each process 
except the master one. To understand and get deeper in each scri-
pt just look for the .txt of each process or see the comments in
each script, they explain a lot of things.
All the pids are reported on the console each time we launch the
program or we open some processes.


