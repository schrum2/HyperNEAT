#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 rom_name"
    exit
fi

ROM=$1

nohup ./experimentalRun roms/$ROM.bin results/$ROM/ 123 10 >> results/$ROM/nohup.out &
