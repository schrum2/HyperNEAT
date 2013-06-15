#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 results_path"
    exit
fi

if [ "$1" == "-f" ]
then
    echo "Force Cleaning $2"
    rm $2/worker* $2/global.err
    exit
fi

for D in $1/*; do
    if [ ! -f $D/generation150.ser.gz ]
    then
        echo -n "[$D]: Not yet finished: "
        ls $D/generation*
    else
        ls -l $D/fitness.* > /dev/null 2>&1
        if [ "$?" = "0" ]
        then
            rm -f $D/fitness.*                            
            echo "{$D} Cleaning fitness files..."
        fi

        ls -l $D/worker* > /dev/null 2>&1
        if [ "$?" = "0" ]
        then
            rm -f $D/worker*
            echo "{$D} Cleaning worker files..."
        fi
        if [ -f $D/global.err ]
        then
            rm -f $D/global.err
            echo "{$D} Cleaning error file..."
        fi
    fi
done


