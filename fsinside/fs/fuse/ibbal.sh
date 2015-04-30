#!/bin/bash

ROOT=./mnt

for i in {1..4}
do
	mkdir $ROOT/dir$i
	for j in {1..4}
	do
		mkdir $ROOT/dir$i/dir$j
		for k in {1..4}
		do
			mkdir $ROOT/dir$i/dir$j/dir$k
		done
	done
done

exit

for i in {1..6}
do
	mkdir $ROOT/dir$i
	for j in {1..82}
	do
		dd if=/dev/urandom of=$ROOT/dir$i/file$j bs=512 count=12
	done
done
