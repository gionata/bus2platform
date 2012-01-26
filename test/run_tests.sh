#!/bin/sh

executable=../src/gap_graph
instancedir=../instances

for inst in $(ls $instancedir/*.txt | sort); do
	dir=$(basename ${inst%.txt})
	rm -rf $dir
	mkdir $dir
	cd $dir
	../$executable ../$inst
	cd ..
done

