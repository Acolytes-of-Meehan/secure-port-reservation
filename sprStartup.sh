#!/bin/bash
#
# Authors: 
#   Ben Ellerby
#   Ray Weiming Luo
#   Evan Ricks
#   Oliver Smith-Denny
#
# Startup script to run the secure port reservation.

echo "starting secure port reservation script..."
echo

echo "compile files..."
gcc -c -g spr.h

for i in *.c
do
    echo $i
    gcc -c -g -Wall $i
done

echo "compile done"
echo

echo "run requester..."
if [ -a requester ]
    then
        ./requester
fi
echo "requester done"
echo

echo "clean up..."
rm -f sender , requester , uds , *.o , *.gch , *# , *~
echo "cleaning done"
echo
