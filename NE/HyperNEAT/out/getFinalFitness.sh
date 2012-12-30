#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 results_dir"
    exit
fi

RESULTSDIR=$1

for D in $RESULTSDIR/*; do
    if [ ! -f $D/nohup.out ]
    then
        echo "[$D]: No nohup.out file!"
    else
        #echo -n "[$D]: "
        grep "Champion fitness" $D/nohup.out | tail -n 1 | awk '{print $3}'
    fi
done

