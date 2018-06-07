#!/bin/bash
CONTIKI=/media/user/Harddisk/birdmac/

echo "Birdmac simulation"
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h

ARRIVAL_RATE=$1
SYNC_PERIOD=$2
SEED_NUMBER=$3
LABEL=$4
SIM_TIME=$5
APP=$6

sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/sim_script/333\_$APP\.csc 
sed -i "s/TIMEOUT([[:digit:]]*);/TIMEOUT($SIM_TIME);/" $CONTIKI/sim_script/333\_$APP\.csc 

DIR=$LABEL\_rate$ARRIVAL_RATE\_sync$SYNC_PERIODd\_seed$SEED_NUMBER

if [ ! -e $DIR ]
then
    mkdir $DIR
fi
cd $DIR

../param.sh $ARRIVAL_RATE $SYNC_PERIOD $SEED_NUMBER $APP

IN_DIR=birdmac
if [ ! -e $IN_DIR ]
then
    mkdir $IN_DIR
fi
cd $IN_DIR
echo "#########################  We are in $PWD  ########################"

HERE=$PWD
cd $CONTIKI/lanada/app_$APP
make clean TARGET=cooja
cd $HERE

if [ ! -e COOJA.testlog ]
then
    java -mx512m -jar $CONTIKI/tools/cooja_$APP/dist/cooja.jar -nogui=$CONTIKI/sim_script/333\_$APP\.csc -contiki="$CONTIKI"
#    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada_orchestra/sim_script/$topology\.csc -contiki="$CONTIKI"
	#	java -mx512m -classpath $CONTIKI/tools/cooja/apps/mrm/lib/mrm.jar: -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0729_$topology\_$LR_range\.csc -contiki="$CONTIKI"
		# ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc -Ddir=$PWD
	#	ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
fi

#sed -i '/last message/d' COOJA.testlog
if [ ! -e report_summary.txt ]
then
    ../../pp_test.sh
fi
cd ../..

echo "Simulation finished"
