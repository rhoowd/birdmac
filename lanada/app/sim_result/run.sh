#!/bin/bash
VAR_ARRIVAL=(600) # lambda
VAR_SYNC_PERIOD=(1200)  # In sec
LABEL="TEST"
#SEED_NUMBER=("1" "2" "3" "4" "5")
SEED_NUMBER=("1")
#SIM_TIME=(10800000) # Default
ASYNC=0 # When run async simulation
APP=1


# Async sim
if [ $ASYNC -eq 1 ]
then
    for seed in "${SEED_NUMBER[@]}"
    do
	for rate in "${VAR_ARRIVAL[@]}"
	do
	    for sync_period in "${VAR_SYNC_PERIOD[@]}"
	    do
		let "SIM_TIME=$sync_period*100*1000"
		./async_run.sh $rate $sync_period $seed "${LABEL}" $SIM_TIME $APP
	    done
	done
    done
fi

# Bridmac
if [ $ASYNC -eq 0 ]
then
    for seed in "${SEED_NUMBER[@]}"
    do
	for rate in "${VAR_ARRIVAL[@]}"
	do
	    for sync_period in "${VAR_SYNC_PERIOD[@]}"
	    do
		let "SIM_TIME=$sync_period*100*1000"
		./bird_run.sh $rate $sync_period $seed "${LABEL}" $SIM_TIME $APP
	    done
	done
    done
fi
