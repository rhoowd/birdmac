#!/bin/bash

for dir in */ ; do
    cd $dir
    for indir in */ ; do
	cd $indir
	if [ -e COOJA.testlog ]
	then
	    echo "sim done "$dir $indir
	else
	    echo "not yet  "$dir $indir
	fi
	cd ..
    done
    cd ..
done
