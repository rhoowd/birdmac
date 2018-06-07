#!/bin/bash

for dir in *; do
    echo "############# $dir ############"
    cd $dir
    echo "duty cycle"
    tail -n 4 parsing/AVG.txt > parsing/duty_temp
    head -n 1 parsing/duty_temp
    rm parsing/duty_temp

    echo "delay"
    cat delay/avg_packet_delay.txt
    # sum_delay=`awk -F ' ' '{sum += $3} END {print sum}' delay/avg_packet_delay.txt`
    # num_delay=`wc -l < "delay/avg_packet_delay.txt"` 
    # echo "@@@ AVR_DELAY"
    # echo "scale=1;$sum_delay/$num_delay"|bc
    # echo "us"

    echo "PRR"
    cat PRR/PRR.txt
    # sum_PRR=`awk -F ' ' '{sum += $2} END {print sum}' PRR/PRR.txt`
    # num_PRR=`wc -l < "PRR/PRR.txt"`
    # echo "@@@ AVR_PRR"
    # echo "scale=3;$sum_PRR/$num_PRR"|bc
    # echo "%"
    cd ..
done
