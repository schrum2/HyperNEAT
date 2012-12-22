#!/bin/bash
# Starts games running sequentially from the specified rom

if [ $# -lt 1 ]
then
    echo "Usage: $0 rom_name"
    exit
fi

let FOUND_ROM=0
let STARTED=0
for D in results/*; do
    rom=${D##*/}

    # Skip all roms before
    if [ $rom = $1 ]
    then
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

        quickRun.sh $rom

        sleep 3

        # Wait until a generation file is present before going on to next ROM
        while true; do
            ls -l $D/generation* > /dev/null 2>&1
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

echo "[quickRunAll.sh] - All games running!"