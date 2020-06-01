#!/bin/bash
./supervisor.o 8 > >(tee logSuper) &
x=$!
sleep 1
for((i=0;i<10;i+=1)); do
	sleep 1
	./client.o 5 8 20 > >(tee -a logClient) &
	./client.o 5 8 20 > >(tee -a logClient) &
done	

for((i=0; i<6;i+=1)); do
	sleep 10
	kill -2 $x
done
kill -2 $x

./misure.sh > >(tee -a logMisure)