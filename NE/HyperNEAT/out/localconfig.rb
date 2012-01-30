# path to script that runs simulator and agent
$run_path = "/u/mhauskn/projects/HyperNEAT/NE/HyperNEAT/out/atari_evaluate"

# time to sleep between condor checks
$sleepTime = 5

# Kill off remaining condor jobs if this many seconds have
# passed without any jobs completing.
$maxWaitTime = 300

# continue to the next iteration when no more than this number 
# of policies have not yet been successfully evaluated
$minJobs = 0.20*$numInds

# kill off remaining condor jobs when no more than this number 
# remain.  keeps slow jobs from choking our system
$minCondorJobs = 0.10*$numInds

# number of instances to run on local computer (iteratively)
$localCount = 0 

# whether or not to store process data in the process directory.  This includes
# log, error, and output from the simulator and agents, and can be really, really big.
# WARNING: only enable this if you're debugging and are running very few agents (approximately 1)
#   and very few iterations (also, approximately 1).  Otherwise, you'll write gigabytes of
#   data in a few seconds, run out of quota, and be generally unhappy.
$logEnabled = false

# which executable will generate the next set of parameters:  controls which method
# is used to optimize parameters.
$path_to_generator = "/u/mhauskn/projects/HyperNEAT/NE/HyperNEAT/out/atari_generate"

# if true, look for a file called "finalpolicy_#{iter}.txt" after each iteration
# and evaluate it locally
$evaluateFinalPolicy = false;
