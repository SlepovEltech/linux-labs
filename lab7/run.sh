#!/bin/sh
rm 1.txt
rm 2.txt
gcc proc1.c -o proc1
gcc proc2.c -o proc2
gcc lab7.c 
./a.out input.txt 1.txt 2.txt
