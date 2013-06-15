#!/bin/bash
# Starts games running sequentially from the specified rom

if [ $# -lt 5 ]
then
    echo "Usage: $0 rom_name seed workers data_path results_path"
    exit
fi

let FOUND_ROM=0
let STARTED=0
STARTROM=$1
SEED=$2
WORKERS=$3
DATAPATH=$4
RESULTSPATH=$5
for D in roms/*; do
    rom=${D##*/} 
    rom=`echo $rom | sed s'/.bin//'`

    # Skip all roms before
    if [ "$rom" = "$STARTROM" ]
    then
        echo "Starting on ROM $rom"
        FOUND_ROM=1
    fi

    if [ $FOUND_ROM = 1 ]
    then
        if [ -f $D/generation150.ser.gz ]
        then
            echo "[$rom] is finished"
            continue
        fi

        echo "Starting $rom..."
        let STARTED=$STARTED+1

        quickRun.sh $rom $SEED $WORKERS $DATAPATH $RESULTSPATH

        sleep 3

        # Wait until a generation file is present before going on to next ROM
        while true; do
            ls -l $RESULTSPATH/$rom/generation* > /dev/null 2>&1
            if [ "$?" = "0" ]
            then
                break
            else
                echo "Waiting for $D to create generational file!"
                sleep 2
            fi
        done
        echo "$STARTED rom(s) started."
    fi
done

if [ $FOUND_ROM = 0 ]
then
    echo "Did not find specified ROM $STARTROM"
else
    echo "[quickRunAll.sh] - All games running!"
fi