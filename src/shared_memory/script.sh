#!/usr/bin/sh

# script for removing all the shared memory in the system 
# sometimes there might be bugs because of reinitializing shared memory
output=$(ipcs -mp | awk '{print $1}' | grep "[0-9]")
echo $output
for value in $output
do
	ipcrm -m $value
done

