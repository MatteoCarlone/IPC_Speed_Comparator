#!/bin/bash

# Running the program and moving the debug.txt from
# the executables folder to a new one dedicated to 
# the logfile.

cd $X
if [[ -d logfile ]] 
then 
	echo "/logfile already exists on your filesystem." 
	rm -r logfile
fi
mkdir logfile
cd executables
./master
cd ..
mv executables/debug.txt logfile
