#!/bin/bash

# This is the install.sh file, you have to run it to install  the program.

# If there's no first argument the bash echoes the instruction to use pro-
# prierly the bash script. Remember to use source! So we can use the envi-
# roment of the shell we're running the .sh on.

if [ $# -eq 0 ]
then
 echo "Usage: source ./install.sh <pathname> ";
 echo "Pay attention to 'source' prefix!";
 exit;
fi

# The if statement is needed to check if the folder actually exists inside 
# our path. If it doesn't it asks the user if he wants to create it.

if [ ! -d $1 ]
then
 echo "Error: Directory $1 DOES NOT exist.";
 while true; do
  read -p "Do you want to create $1 directory? [Y/n] " yn
  case $yn in
   [Yy]* ) mkdir $1; break;;
   [Nn]* ) exit;;
   * ) "Please, answer Y for yes or n for no.";;
  esac
 done
fi

# Now we actually begin the real installation of the program by unzipping 
# the src.zip. We also export the <path> of the folder to use it in anot-
# her file bash.

echo "Begin program installation on $1 ... ";
export X=$1
unzip src.zip -d $1;

# Now we compile all the .c files and we put the executables inside a new 
# folder called executables.

echo "Begin sources' compilation ...";

mkdir $1/executables;
gcc $1/dmaster/master.c -o $1/executables/master;
gcc $1/dup/up.c -o $1/executables/up;
gcc $1/dnp/np.c -o $1/executables/np;
gcc $1/dsocket/socket.c -o $1/executables/socket;
gcc -pthread -lrt $1/dcb/cb.c -o $1/executables/cb;

echo "You can run the program with ./run.sh ";

