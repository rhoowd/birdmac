#!/bin/bash

JOONKI=0

if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/contiki-3.0-async/
else
    CONTIKI=~/Desktop/contiki-3.0-async/
fi

echo "Async simulation"
#sed -i 's/\#define DUAL_RADIO 1/\#define DUAL_RADIO 0/g' $CONTIKI/platform/cooja/contiki-conf.h
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h

topology=$1
TRAFFIC_MODEL=$2
PERIOD=$3
ARRIVAL_RATE=$4
LABEL=$5
CHECK=$6
SEED_NUMBER=$7
TSCH=$8
APP=$9
SIM_TIME=${10}

sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada_async/sim_script/$topology\_async\_$APP\.csc 
sed -i "s/TIMEOUT([[:digit:]]*);/TIMEOUT($SIM_TIME);/" $CONTIKI/lanada_async/sim_script/$topology\_async\_$APP\.csc 

if [ $TRAFFIC_MODEL -eq 0 ]
then
    DIR=$LABEL\_topo$topology\_traffic$TRAFFIC_MODEL\_period$PERIOD\_seed$SEED_NUMBER
else
    DIR=$LABEL\_topo$topology\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_seed$SEED_NUMBER
fi
if [ ! -e $DIR ]
then
    mkdir $DIR
fi
cd $DIR

../async_param.sh $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $CHECK $APP

IN_DIR=tsch$TSCH\_check$CHECK
if [ ! -e $IN_DIR ]
then
    mkdir $IN_DIR
#    mkdir sr\_strobe$STROBE_CNT\_$LR_range\_$CHECK\_rou$ROUTING_NO_ENERGY
fi
cd $IN_DIR
#cd sr\_strobe$STROBE_CNT\_$LR_range\_$CHECK\_rou$ROUTING_NO_ENERGY
echo "#########################  We are in $PWD  ########################"

HERE=$PWD
#cd $CONTIKI/lanada_async
cd $CONTIKI/lanada_$APP
make clean TARGET=cooja
cd $HERE

if [ ! -e COOJA.testlog ]
then
    java -mx512m -jar $CONTIKI/tools/cooja_$APP/dist/cooja.jar -nogui=$CONTIKI/lanada_async/sim_script/$topology\_async\_$APP\.csc -contiki="$CONTIKI"
#    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada_async/sim_script/$topology\_async\.csc -contiki="$CONTIKI"
	#	java -mx512m -classpath $CONTIKI/tools/cooja/apps/mrm/lib/mrm.jar: -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0729_$topology\_$LR_range\.csc -contiki="$CONTIKI"
		# ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc -Ddir=$PWD
	#	ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
fi
# else # If MRM mode
# 	if [ ! -e COOJA.testlog ]
# 	then
# 		cd $CONTIKI/tools/cooja_mrm$MRM 
# 		if [ $ONLY_LONG -eq 0 ]
# 		then
# 			ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc
# 		else
# 			ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_L\.csc
# 		fi
# 		mv build/COOJA.testlog $HERE
# 		cd $HERE
# 	fi
# fi

sed -i '/last message/d' COOJA.testlog
if [ ! -e report_summary.txt ]
then
    ../../pp_test.sh
fi
cd ../..

echo "Simulation finished"

