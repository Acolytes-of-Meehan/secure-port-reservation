#!/bin/bash
#
# Authors: 
#   Ben Ellerby
#   Ray Weiming Luo
#   Evan Ricks
#   Oliver Smith-Denny
#
# Startup script to run the secure port reservation.

echo "Starting secure port reservation script..."
echo

################################################################

echo "Compiling C program files..."
for i in *.c
do
    echo $i
    gcc -c -g -Wall $i
done

echo "Compiling C program files... DONE."
echo

################################################################

echo -n "Creating daemon... "
gcc -o -g -Wall sprd.o parse_config.o tokenizer.o linked_list.o -o sprd
echo "DONE."
echo

################################################################

echo -n "Creating socket test... "
gcc -o -g -Wall finalRequester.o secure_bind.o secure_close.o -o finalRequester
echo "DONE."
echo

################################################################
