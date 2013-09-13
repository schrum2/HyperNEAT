import os, subprocess

def submitCondorJob(condorFile):
    output = subprocess.Popen(["condor_submit","-verbose",condorFile],stdout=subprocess.PIPE).communicate()[0]

    # Get the process ID for this job so we can monitor it
    s = output.find('** Proc ')+8
    procID = output[s:output.find(':\n',s)]
    return procID


def getCurrentGen(resultsDir):
    # Detect the current generation
    currentGeneration = -1
    for f in os.listdir(resultsDir):
        if f.startswith('generation') and 'tmp' not in f and 'bak' not in f:
            genNumber = int(f[len('generation'):-len('.ser.gz')])
            currentGeneration = max(currentGeneration, genNumber)
    return currentGeneration

def getPIDStatus(pid, condor_q):
    # Returns the status of a given pid in the condor_q output
    for line in condor_q.split('\n'):
        if line.startswith(str(pid)):
            return line.split()[5]
    return None

def condor_q():
    # Gets the output of the condor_q command
    return subprocess.Popen(["condor_q","mhauskn"],stdout=subprocess.PIPE).communicate()[0]

def condorKillPID(pid):
    # Kills a pid on condor
    return subprocess.Popen(["condor_rm",str(pid)],stdout=subprocess.PIPE).communicate()[0]
