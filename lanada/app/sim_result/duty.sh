#!/bin/bash

for DIR in $1*
do
	if [ -d $DIR ]
	then
		echo "############# $DIR ###############"
		cd $DIR
		for dir in *
		do
			cd $dir
			duty=`tail -4 parsing/AVG.txt | head -1`
			echo "$dir : $duty"
			cd ..
		done
		echo 
	fi
	cd ..
done


