#!/bin/bash

#TOPOLOGY=("grid_36" "tree_c2_31" "tree_c3_40" "random_50")
TOPOLOGY=("xxx")

#for topo in "${TOPOLOGY[@]}" do
    for dir in */ ; do
#	if [ -d $dir -a -n "`echo "$dir" | grep $topo`"]
#	then
	    cd $dir
	    for indir in */ ; do
		cd $indir
		sed -i '/last message/d' ./COOJA.testlog
		if [ ! -e report_summary.txt ]
		then
		    ../../pp_test.sh
		fi
		cd ..
	    done
	    cd ..
#	fi
    done
#done
