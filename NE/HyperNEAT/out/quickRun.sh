#!/bin/bash

if [ $# -lt 5 ]
then
    echo "Usage: $0 rom_name seed workers data_path results_path"
    exit
fi

ROM=$1
SEED=$2
WORKERS=$3
DATAPATH=$4
RESULTSPATH=$5

mkdir -p $RESULTSPATH/$ROM
nohup ./experimentalRun roms/$ROM.bin $RESULTSPATH/$ROM/ $SEED $WORKERS $DATAPATH >> $RESULTSPATH/$ROM/nohup.out &
