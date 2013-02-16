#!/bin/bash

if [ $# -lt 4 ]
then
    echo "Usage: $0 rom_name seed workers data_path"
    exit
fi

ROM=$1
SEED=$2
WORKERS=$3
DATAPATH=$4

nohup ./experimentalRun roms/$ROM.bin results/$ROM/ $SEED $WORKERS $DATAPATH >> results/$ROM/nohup.out &
