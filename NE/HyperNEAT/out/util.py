import os, subprocess, time

def submitCondorJob(condorFile):
    output = subprocess.Popen(["condor_submit","-verbose",condorFile],stdout=subprocess.PIPE).communicate()[0]

    # Get the process ID for this job so we can monitor it
    s = output.find('** Proc ')+8
    procID = output[s:output.find(':\n',s)]
    return procID

# Detect the current generation
def getCurrentGen(resultsDir, CMAES_exp=False):
    if CMAES_exp:
        return getCurrentCMAESGen(resultsDir)
    else:
        currentGeneration = -1
        for f in os.listdir(resultsDir):
            if f.startswith('generation') and 'tmp' not in f and 'bak' not in f:
                genNumber = int(f[len('generation'):-len('.ser.gz')])
                currentGeneration = max(currentGeneration, genNumber)
        return currentGeneration

# Detect the current generation CMA-ES is on
def getCurrentCMAESGen(resultsDir):
    currentGeneration = -1
    for f in os.listdir(resultsDir):
        if f.startswith('paramswritten_'):
            genNumber = int(f[len('paramswritten_'):-len('.txt')])
            currentGeneration = max(currentGeneration, genNumber)
    return currentGeneration

def getPIDStatus(pid, condor_q):
    # Returns the status of a given pid in the condor_q output
    for line in condor_q.split('\n'):
        if line.startswith(str(pid)):
            return line.split()[5]
    return None

# Gets the output of the condor_q command
def condor_q():
    out = subprocess.Popen(["condor_q","mhauskn"],stdout=subprocess.PIPE).communicate()[0]
    if not out or out.find('Failed to fetch ads') >= 0:
        return None
    return out

def condorKillPID(pid):
    # Kills a pid on condor
    return subprocess.Popen(["condor_rm",str(pid)],stdout=subprocess.PIPE).communicate()[0]

# Starts the generator running - either CMAES or normal generator
def startGenerator(generatorNum, executable, resultsDir, dataFile,
                   numIndividuals, numGenerations, seed, rom):
    if dataFile.endswith('AtariCMAExperiment.dat'):
        # First create the initial generation
        firstGenPath = os.path.join(resultsDir,"generation0.ser.gz")
        if not os.path.exists(firstGenPath):
            subprocess.check_call(["./" + executable,
                                   "-I", dataFile,
                                   "-R", str(seed),
                                   "-O", firstGenPath,
                                   "-G", rom])
        # Then start CMA-ES
        return startCMAESGenerator(generatorNum, executable, resultsDir, dataFile,
                   numIndividuals, numGenerations, seed, rom)
    else:
        return startNormalGenerator(generatorNum, executable, resultsDir, dataFile,
                   numIndividuals, numGenerations, seed, rom)

# Starts a worker running - either CMAES or normal worker
def startWorker(workerNum, executable, resultsDir, dataFile, numIndividuals,
                numGenerations, seed, rom):
    if dataFile.endswith('AtariCMAExperiment.dat'):
        return startCMAESWorker(workerNum, executable, resultsDir, dataFile, numIndividuals,
                numGenerations, seed, rom)
    else:
        return startNormalWorker(workerNum, executable, resultsDir, dataFile, numIndividuals,
                numGenerations, seed, rom)


# Submits a condor job which starts a worker running
def startNormalWorker(workerNum, executable, resultsDir, dataFile, numIndividuals, numGenerations, seed, rom):
    cOutFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".out")
    cErrFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".err")
    cLogFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".log")
    confStr = "\
    Output = " + cOutFile +"\n\
    Error = " + cErrFile +"\n\
    Log = " + cLogFile + "\n\
    universe = vanilla\n\
    getenv = true\n\
    Executable = " + "/lusr/bin/python" + "\n\
    Arguments = worker.py -e "+ executable + " -r " + resultsDir + " -d " + dataFile + " -n " + str(numIndividuals) + " -g " + str(numGenerations) + " -R " + str(seed) + " -G " + rom + "\n\
    Requirements = Arch == \"x86_64\" && InMastodon\n\
    +Group=\"GRAD\"\n\
    +Project=\"AI_ROBOTICS\"\n\
    +ProjectDescription=\"HyperNEAT Atari Game Playing Worker.\"\n\
    Queue\n"

    # Submit the condor job
    condorFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".submit")
    condorSubmitPipe = open(condorFile,'w')
    condorSubmitPipe.write(confStr)
    condorSubmitPipe.close()
    procID = submitCondorJob(condorFile)

    # Wait for this job to show up in the condor_q before returning
    for i in range(12):
        time.sleep(10)
        if getPIDStatus(procID, condor_q()) != None:
            return procID

    print 'Failed to start worker.'
    return -1


# Submits a condor job which starts the generator running
def startNormalGenerator(generatorNum, executable, resultsDir, dataFile,
                   numIndividuals, numGenerations, seed, rom):
    cOutFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".out")
    cErrFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".err")
    cLogFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".log")
    confStr = "\
    Output = " + cOutFile +"\n\
    Error = " + cErrFile +"\n\
    Log = " + cLogFile + "\n\
    universe = vanilla\n\
    getenv = true\n\
    Executable = " + "/lusr/bin/python" + "\n\
    Arguments = generator.py -e "+ executable + " -r " + resultsDir + " -d " + dataFile + " -n " + str(numIndividuals) + " -g " + str(numGenerations) + " -R " + str(seed) + " -G " + rom + "\n\
    Requirements = Arch == \"x86_64\" && InMastodon\n\
    +Group=\"GRAD\"\n\
    +Project=\"AI_ROBOTICS\"\n\
    +ProjectDescription=\"HyperNEAT Atari Game Playing Generator.\"\n\
    Queue\n"

    # Submit the condor job
    condorFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".submit")
    condorSubmitPipe = open(condorFile,'w')
    condorSubmitPipe.write(confStr)
    condorSubmitPipe.close()
    procID = submitCondorJob(condorFile)

    # Wait for this job to show up in the condor_q before returning
    for i in range(12):
        if getPIDStatus(procID, condor_q()) != None:
            return procID
        time.sleep(10)

    print 'Failed to start generator.'
    return -1

def startCMAESGenerator(generatorNum, executable, resultsDir, dataFile,
                   numIndividuals, numGenerations, seed, rom):
    cOutFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".out")
    cErrFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".err")
    cLogFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".log")
    inputFile = os.path.join(resultsDir,"FTNeatCMA_Input.txt")

    
    paramsGen = getCurrentCMAESGen(resultsDir)
    if paramsGen >= 0:
        confStr = "\
        Output = " + cOutFile +"\n\
        Error = " + cErrFile +"\n\
        Log = " + cLogFile + "\n\
        universe = vanilla\n\
        getenv = true\n\
        Executable = " + "/usr/bin/java" + "\n\
        Arguments = -Xmx16192m -cp /u/mhauskn/projects/frameworks/cma/java cma.CMAMain " + resultsDir + " " + str(numGenerations) + " -c " + str(paramsGen) + "\n\
        Requirements = Arch == \"x86_64\" && InMastodon\n\
        +Group=\"GRAD\"\n\
        +Project=\"AI_ROBOTICS\"\n\
        +ProjectDescription=\"HyperNEAT Atari Game Playing Generator.\"\n\
        Queue\n"
    else:
        confStr = "\
        Output = " + cOutFile +"\n\
        Error = " + cErrFile +"\n\
        Log = " + cLogFile + "\n\
        universe = vanilla\n\
        getenv = true\n\
        Executable = " + "/usr/bin/java" + "\n\
        Arguments = -Xmx16192m -cp /u/mhauskn/projects/frameworks/cma/java cma.CMAMain " + resultsDir + " " + str(numGenerations) + " " + str(numIndividuals) + " " + inputFile + "\n\
        Requirements = Arch == \"x86_64\" && InMastodon\n\
        +Group=\"GRAD\"\n\
        +Project=\"AI_ROBOTICS\"\n\
        +ProjectDescription=\"HyperNEAT Atari Game Playing Generator.\"\n\
        Queue\n"
        

    # Submit the condor job
    condorFile = os.path.join(resultsDir,"generator" + str(generatorNum) + ".submit")
    condorSubmitPipe = open(condorFile,'w')
    condorSubmitPipe.write(confStr)
    condorSubmitPipe.close()
    procID = submitCondorJob(condorFile)

    # Wait for this job to show up in the condor_q before returning
    for i in range(12):
        if getPIDStatus(procID, condor_q()) != None:
            return procID
        time.sleep(10)

    print 'Failed to start generator.'
    return -1

def startCMAESWorker(workerNum, executable, resultsDir, dataFile, numIndividuals, numGenerations, seed, rom):
    cOutFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".out")
    cErrFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".err")
    cLogFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".log")
    confStr = "\
    Output = " + cOutFile +"\n\
    Error = " + cErrFile +"\n\
    Log = " + cLogFile + "\n\
    universe = vanilla\n\
    getenv = true\n\
    Executable = " + "/lusr/bin/python" + "\n\
    Arguments = CMAESWorker.py -e "+ executable + " -r " + resultsDir + " -d " + dataFile + " -n " + str(numIndividuals) + " -g " + str(numGenerations) + " -R " + str(seed) + " -G " + rom + "\n\
    Requirements = Arch == \"x86_64\" && InMastodon\n\
    +Group=\"GRAD\"\n\
    +Project=\"AI_ROBOTICS\"\n\
    +ProjectDescription=\"HyperNEAT Atari Game Playing Worker.\"\n\
    Queue\n"

    # Submit the condor job
    condorFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".submit")
    condorSubmitPipe = open(condorFile,'w')
    condorSubmitPipe.write(confStr)
    condorSubmitPipe.close()
    procID = submitCondorJob(condorFile)

    # Wait for this job to show up in the condor_q before returning
    for i in range(12):
        time.sleep(10)
        if getPIDStatus(procID, condor_q()) != None:
            return procID

    print 'Failed to start worker.'
    return -1
