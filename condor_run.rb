#!/usr/bin/env ruby

# This script automates runs of optimization algorithms and their 
# parameter generator for sim-3d robosoccer.
#
# Usage: condor_run.rb <experiment_path> <iters> <pop_size>
#
#  * <experiment_path>:  where to store results and logs of the experiment.  
#                        will be created if it doesn't exist.
#  * <iters>:            total number of iterations to perform 
#                        will be created if it doesn't exist.
#  * <pop_size>:	 number of policies to try per iteration
#  * [config_file_path]: optional path to config parameter file
#
# Author: yinon@cs.utexas.edu (Yinon Bentor) [base code]
# Author: patmac@cs.utexas.edu (Patrick MacAlpine) [averaging, lots of improvements]

# Parse command line arguments 

if (ARGV.size < 4)
  STDERR.print "Usage: #{$0} <experiment_path> <iters> <pop_size> [config_file_path]\n"
  exit(1)
end

$experimentbase = File.expand_path(ARGV[0])

# number of iterations
$maxIter = ARGV[1].to_i 

# number of policies to try per iteration
$numInds = ARGV[2].to_i

if (ARGV.size >= 5)
  $configpath = File.expand_path(ARGV[4])
else
  $configpath = File.dirname(__FILE__) + "/localconfig.rb"
end


# Some parameters:

# Read in user or experiment-specific variables
if (File.exists?($configpath)) 
  require ($configpath)
else
  STDERR.print "Could not find the configuration file #{$configpath}.  Exiting!\n"
  exit(1)
end

config_parameters = ["$run_path", "$sleepTime", "$maxWaitTime", "$minJobs", "$minCondorJobs", "$localCount",
  "$logEnabled", "$path_to_generator", "$evaluateFinalPolicy"]

config_parameters.each do |var|
  if (eval("defined?(#{var})").nil?)
    STDERR.print "Could not find variable #{var} in the configuration file #{$configpath}.  Exiting!\n"
    exit(1)
  end
end

# condor user
$user = `whoami`.strip

# if true, look for a file called "finalpolicy_#{iter}.txt" after each iteration
# and evaluate it locally
$evaluateFinalPolicy = false;

# Hostname is used to export the display on the remote machine back to the
# master node
require 'socket'
$hostname = Socket.gethostname

# ---------------------------------------


#First, let's clear off existing condor jobs
system("condor_rm #{$user}")

#Insecure, but I don't know a way around this:
system("xhost +")

# parameters:
#   gen:      integer representing the iteration number, starting with 1
#   inds:     array of integers containing the policy numbers within the iteration to evaluate
#   run:        the number of the current run we're on for averaging
#   retries:  the number of retries that we've seen so far.  used to log to a different log file
#   runlocal: an array of integers containing the policy numbers to run locally instead on condorContents
def run_on_condor(gen, inds, run, retries=0, runlocal = false) 

  #paramsFile = $experimentbase + "/results/params_#{gen}_i_\$(Process).txt"
  #valueFile = $experimentbase + "/results/value_#{gen}_i_\$(Process).txt"
  
  inds = inds - runlocal  # don't run the local jobs on condor too
  condorContents = <<END_OF_CONDORFILE;

Executable = #{$run_path}
Universe = vanilla
Environment = ONCONDOR=true
Getenv = true
Requirements = Lucid

+Group = "GRAD"
+Project = "AI_ROBOTICS"
+ProjectDescription = "HyperNEAT evaluations for class project in CS394n"

Input = /dev/null


END_OF_CONDORFILE
	
  inds.each do |i|
    
    dataFile = $experimentbase + "/data/AtariExperiment.dat"
    populationFile = $experimentbase + "/results/generation#{gen}.xml.gz"
    fitnessFile = $experimentbase + "/results/fitness.#{gen}.#{i}";
    individualId = $experimentbase + "/results/fitness.#{gen}.#{i}";
   
    condorContents = condorContents + "\n"

    if ($logEnabled)
       condorContents = condorContents + 
       "Error = #{$experimentbase}/process/error-#{gen}-#{i}-#{retries}.err\n" +
       "Output = #{$experimentbase}/process/out-#{gen}-#{i}-#{retries}.out\n" + 
       "Log = #{$experimentbase}/process/log-#{gen}-#{i}-#{retries}.log\n"
    else 
       condorContents = condorContents +
      "Error = /dev/null\n" + 
      "Output = /dev/null\n" + 
      "Log = /dev/null\n"
    end
    condorContents = condorContents + "arguments = #{paramsFile} #{valueFileRun}\n" + 
      "Queue 1";
  end

    
  #submit the job:
  if (inds.size > 0)
    print "Submitting job for evaluation of generation #{gen} run #{run}\n"
    condorSubmitPipe = open("|condor_submit", 'w');
    condorSubmitPipe.write(condorContents)
    condorSubmitPipe.close
    sleep 1
  end

  runlocal.each do |i|
    print "\nRunning #{i} locally:\n"
    # Run the ones that need to be run locally locally
    localParamsFile = $experimentbase + "/results/params_#{gen}_i_#{i}.txt";
    localValueFileRun = $experimentbase + "/results/run_#{gen}_i_#{i}_r_#{run}.txt";
    
    # run our local one while we wait
    system("#{$run_path} #{localParamsFile} #{localValueFileRun}")
  end
  
  #wait for jobs to return
  jobsRemain = $minCondorJobs + 1
  
  lastJobsRemain = jobsRemain
  totalWaitTime = 0
  while(jobsRemain > $minCondorJobs)
    condorQ = open("|condor_q #{$user}", 'r')
    lastLine = condorQ.readlines.last
    lastLine =~ /([0-9]+) jobs/
    jobsRemain = $1.to_i
    print "Waiting on jobs: #{jobsRemain} left.. Sleep for #{$sleepTime}\n"
    condorQ.close
    sleep $sleepTime
    if (jobsRemain != lastJobsRemain)
      totalWaitTime = 0
    end    
    lastJobsRemain = jobsRemain
    totalWaitTime = totalWaitTime + $sleepTime
    if (totalWaitTime > $maxWaitTime)
      break
    end
    
  end
  # clean up remaining jobs
  system("condor_rm #{$user}")
  sleep $sleepTime
end

#make sure that the 'process' and 'results' directories exist under $experimentbase
Dir.mkdir($experimentbase) unless File.exists?($experimentbase)

Dir.mkdir($experimentbase + "/process") unless File.exists?($experimentbase + "/process")
Dir.mkdir($experimentbase + "/results") unless File.exists?($experimentbase + "/results")

#generate initial pop first:
system("#{$path_to_generator} #{$experimentbase}/results 0 #{$numInds}")
until(File.exists?("#{$experimentbase}/results/paramswritten_1.txt")) 
 print "Waiting on paramswritten_1.txt\n"
 sleep 1 
end

(1..$maxIter).each do |gen|
  $numAvgRuns.times do |r|  
    print "\n\n"
  
    inds = (0...$numInds).to_a
    run_on_condor(gen, inds, r, 0, inds.slice(0, $localCount))
  
    # next, see if all values files have been created, if not, re-run until they are!
    found = []
    remaining = []
    retries = 0

    while (found.size == 0 || remaining.size > $minJobs)
      retries = retries + 1
      Dir[$experimentbase + "/results/run_#{gen}_i_*_r_#{r}.txt"].each do |file|
        foo = false 
        file =~ /i_([0-9]+)_r_([0-9]+).txt$/
        found << $1.to_i
      end
      remaining = ((0..$numInds-1).to_a - found)
      
      if (remaining.size>$minJobs)
        print "Re-running #{remaining.size} failed policies: [" + remaining.join(" ") + "]:\n"
        local = remaining.slice(0,$localCount)
        run_on_condor(gen, remaining, r, retries, local)
      end
    end
  end

  ######################################
  ######################################
  # Code to average atomic fitness evaluations.

  (0..$numInds-1).each do |i|

    localValueFile = $experimentbase + "/results/value_#{gen}_i_#{i}.txt";

    sum = 0
    count = 0

    $numAvgRuns.times do |r|			
      localValueFileTemp = $experimentbase + "/results/run_#{gen}_i_#{i}_r_#{r}.txt";
      if(File.exists?(localValueFileTemp)) then
        v = File.new(localValueFileTemp).read.strip
        unless (v.empty?) then
          sum = sum + v.to_f
          count = count + 1
      else
          print "Error: File #{localValueFileTemp} is empty!\n"
        end
      end
    end

    if(count >= 1) then
      avg = sum / count
      outFile = File.new(localValueFile, "w")
      outFile.puts(avg)
      outFile.close()
    end

  end

  ######################################
  ######################################
  ######################################


  #now generate a new pop:
  system("#{$path_to_generator} #{$experimentbase}/results #{gen} #{$numInds}")
  File.open("#{$experimentbase}/results/valuationdone_#{gen}.txt", "w") {|f| f.write("")}
  
  # now wait until next parameters are written before continuing
  until(File.exists?("#{$experimentbase}/results/paramswritten_#{gen+1}.txt")) 
    break if gen == $maxIter 
    print "Waiting on paramswritten_#{gen+1}.txt\n"
    sleep 1 
  end 

  # for policy gradient, etc, evaluate the final policy for the iteration:
  if $evaluateFinalPolicy
    print "Evaluating finalpolicy_#{gen}:\n"
    finalPolicyFile = "#{$experimentbase}/results/finalpolicy_#{gen}.txt"
    finalPolicyValueFile = "#{$experimentbase}/results/finalpolicy_value_#{gen}.txt"
    
    system("#{$run_path} #{finalPolicyFile} #{finalPolicyValueFile}")
  end
end

system("xhost -")
#clean up before exiting
system("condor_rm #{$user}")
