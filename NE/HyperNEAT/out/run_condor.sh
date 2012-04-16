#!/bin/bash

LOG="expr.log"
date > $LOG
echo "Starting Experimental Runs..."
echo "Writing to logfile $LOG"
echo "Starting Experimental Runs..." >> $LOG
echo "Current Git CheckPt: " >> $LOG
git rev-parse HEAD >> $LOG

# Experiments Here
for i in {1..1}
do
	./condor_run.rb /u/mhauskn/projects/HyperNEAT/NE/HyperNEAT/out/ 250 100 >> $LOG
done

echo "Experiments Finished!"
echo "Experiments Finished!" >> $LOG
date >> $LOG
