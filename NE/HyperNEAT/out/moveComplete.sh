#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 results_path dest"
    echo "$0 moves completed games to the destination path"
    exit
fi

RESDIR=$1
DEST=$2

for D in $RESDIR/*; do
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
        if [ -f $D/*.tmp ]
        then
            rm -f $D/*.tmp
            echo "{$D} Cleaning tmp files..."
        fi
        echo "Moving ${D} to $DEST/${D}"
        mv ${D} $DEST
    fi
done


