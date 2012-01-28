#!/bin/sh

executable=../src/gap_graph
instancedir=../instances

for inst in $(ls $instancedir/istanza_reale_feasible_no10_1*.txt | sort); do
	dir=$(basename ${inst%.txt})
	rm -rf $dir
	mkdir $dir
	cd $dir
	../$executable ../$inst
	cd ..
done

