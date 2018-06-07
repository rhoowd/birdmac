#!/bin/bash

TSCH=1 # Whether Async(0) or TSCH(1)
ORCHESTRA=1 # Whether Minimal(0) or Orchestra(1)
RBS_SBS=1 # Whether RBS(0) or SBS(1)
TRAFFIC=0 # Whether Periodic(0) or Poisson(1)
ADAPTIVE_MODE=1 # Whether basic(0) or adaptive(1)
VAR_PERIOD=(30) # T
VAR_ARRIVAL=(10) # lambda
VAR_TOPOLOGY=("tree_c4_21") # tree_c4_21 grid_36 random_50 child_4
LABEL="MB5"
SEED_NUMBER=("1" "2" "3" "4" "5")
VAR_N_PBS=("2") # Hard coded n-PBS
VAR_N_SF=("1" "2") # Hard coded n-SF
VAR_CHECK_RATE=(8)
VAR_UNICAST_PERIOD=(11)
VAR_MINIMAL_PERIOD=(7)
SIM_TIME=(10800000)
APP=2
BOTH_TRAFFIC=0
MICROBENCH=1


# Async sim

if [ $TSCH -eq 0 ]
then
    if [ $TRAFFIC -eq 0 ]
    then
	for seed in "${SEED_NUMBER[@]}"
	do
	    for period in "${VAR_PERIOD[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for check in "${VAR_CHECK_RATE[@]}"
		    do
			./async_run.sh $topology $TRAFFIC $period 0 "${LABEL}" $check $seed $TSCH $APP $SIM_TIME
		    done
		done
	    done
	done
    else
	for seed in "${SEED_NUMBER[@]}"
	do
	    for arrival in "${VAR_ARRIVAL[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for check in "${VAR_CHECK_RATE[@]}"
		    do
			./async_run.sh $topology $TRAFFIC 0 $arrival "${LABEL}" $check $seed $TSCH $APP $SIM_TIME
		    done
		done
	    done
	done
    fi
fi

# TSCH sim
if [ $TSCH -eq 1 ]
then
    if [ $TRAFFIC -eq 0 ]
    then
	for seed in "${SEED_NUMBER[@]}"
	do
	    for period in "${VAR_PERIOD[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for n_pbs in "${VAR_N_PBS[@]}"
		    do
			for n_sf in "${VAR_N_SF[@]}"
			do
			    for check in "${VAR_CHECK_RATE[@]}"
			    do
				for uni in "${VAR_UNICAST_PERIOD[@]}"
				do
				    for mini in "${VAR_MINIMAL_PERIOD[@]}"
				    do
					if [ $MICROBENCH -eq 0 ]
					then
					    if [ $period = 10 ]
					    then
				    		SIM_TIME=10800000
					    elif [ $period = 30 ]
					    then
				    		SIM_TIME=32400000
					    fi
					fi
					./tsch_run.sh $topology $TRAFFIC $period 0 "${LABEL}" $check $seed $TSCH $ORCHESTRA $RBS_SBS $ADAPTIVE_MODE $n_pbs $n_sf $uni $mini $APP $SIM_TIME
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    fi
    if [ $BOTH_TRAFFIC -eq 1 ]
    then
	TRAFFIC=1
	LABEL="G2"
    fi
    if [ $TRAFFIC -eq 1 ]
    then
	for seed in "${SEED_NUMBER[@]}"
	do
	    for arrival in "${VAR_ARRIVAL[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for n_pbs in "${VAR_N_PBS[@]}"
		    do
			for n_sf in "${VAR_N_SF[@]}"
			do
			    for check in "${VAR_CHECK_RATE[@]}"
			    do
				for uni in "${VAR_UNICAST_PERIOD[@]}"
				do
				    for mini in "${VAR_MINIMAL_PERIOD[@]}"
				    do
					if [ $MICROBENCH -eq 0 ]
					then
					    if [ $arrival = 1 ]
					    then
				    		SIM_TIME=3600000
					    elif [ $arrival = 5 ]
					    then
				    		SIM_TIME=7200000
					    elif [ $arrival = 10 ]
					    then
				    		SIM_TIME=7200000
					    elif [ $arrival = 25 ]
					    then
				    		SIM_TIME=10800000
					    elif [ $arrival = 50 ]
					    then
				    		SIM_TIME=18000000
					    fi
					fi
					./tsch_run.sh $topology $TRAFFIC 0 $arrival "${LABEL}" $check $seed $TSCH $ORCHESTRA $RBS_SBS $ADAPTIVE_MODE $n_pbs $n_sf $uni $mini $APP $SIM_TIME
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    fi
fi
