#!/bin/sh
exec=main

rm $exec 

gcc -o $exec $exec.c -Wall -lGL -lm -lpthread -ldl -lrt -lX11

./$exec

vlc output.mp4