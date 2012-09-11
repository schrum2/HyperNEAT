#                 _ww   _a+"D
#          y#,  _r^ # _*^  y`
#         q0 0 a"   W*`    F   ____
#      ;  #^ Mw`  __`. .  4-~~^^`
#     _  _P   ` /'^           `www=.
#   , $  +F    `                q
#   K ]                         ^K`
# , #_                . ___ r    ],
# _*.^            '.__dP^^~#,  ,_ *,
# ^b    / _         ``     _F   ]  ]_
#  '___  '               ~~^    ]   [
#  :` ]b_    ~k_               ,`  yl
#    #P        `*a__       __a~   z~`
#    #L     _      ^------~^`   ,/
#     ~-vww*"v_               _/`
#             ^"q_         _x"
#              __#my..___p/`mma____
#          _awP",`,^"-_"^`._ L L  #
#        _#0w_^_^,^r___...._ t [],"w
#       e^   ]b_x^_~^` __,  .]Wy7` x`
#        '=w__^9*$P-*MF`      ^[_.=
#            ^"y   qw/"^_____^~9 t
#              ]_l  ,'^_`..===  x'
#               ">.ak__awwwwWW###r
#                 ##WWWWWWWWWWWWWW__
#                _WWWWWWMM#WWWW_JP^"~-=w_
#      .____awwmp_wNw#[w/`     ^#,      ~b___.
#       ` ^^^~^"W___            ]Raaaamw~`^``^^~
#                 ^~"~---~~~~~~`

import argparse, os, subprocess, time, sys

# Submits a condor job which starts a worker running
def startWorker(workerNum, executable, resultsDir, dataFile, numIndividuals, numGenerations, rom):
    cOutFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".out")
    cErrFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".err")
    cLogFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".log")
    confStr = "\
    Output = " + cOutFile +"\n\
    Error = " + cErrFile +"\n\
    Log = " + cLogFile + "\n\
    universe = vanilla\n\
    Executable = " + "/lusr/bin/python" + "\n\
    Arguments = worker.py -e "+ executable + " -r " + resultsDir + " -d " + dataFile + " -n " + str(numIndividuals) + " -g " + str(numGenerations) + " -G " + rom + "\n\
    Requirements = Lucid && (Arch == \"x86_64\" || Arch==\"INTEL\") && (1024 * Memory >= 10)\n\
    +Group=\"GRAD\"\n\
    +Project=\"AI_ROBOTICS\"\n\
    +ProjectDescription=\"HyperNEAT Atari Game Playing.\"\n\
    Queue\n"

    # Submit the condor job
    condorFile = os.path.join(resultsDir,"worker" + str(workerNum) + ".submit")
    condorSubmitPipe = open(condorFile,'w')
    condorSubmitPipe.write(confStr)
    condorSubmitPipe.close()
    output = subprocess.Popen(["condor_submit","-verbose",condorFile], stdout=subprocess.PIPE).communicate()[0]

    # Get the process ID for this job so we can monitor it
    s = output.find('** Proc ')+8
    procID = output[s:output.find(':\n',s)]
    return procID

parser = argparse.ArgumentParser(description='Creates condor worker jobs who execute atari games.')
parser.add_argument('-e', metavar='atari_evaulate', required=True,
                    help='This should point to atari_evaluate executable.')
parser.add_argument('-p', metavar='atari_generate', required=True,
                    help='The number of workers devoted to running this game.')
parser.add_argument('-r', metavar='results-dir', required=True,
                    help='This should point to the directory that results are being accumulated in.')
parser.add_argument('-d', metavar='data-file', required=True,
                    help='This should point to the AtariExperiment.dat file')
parser.add_argument('-n', metavar='num-individuals', required=True, type=int,
                    help='This is the number of individuals in each generation.')
parser.add_argument('-g', metavar='num-generations', required=True, type=int,
                    help='This is the number of generats before the run is complete.')
parser.add_argument('-G', metavar='rom-file', required=True,
                    help='This should point to the rom to be run.')
parser.add_argument('-w', metavar='num-workers', required=True, type=int,
                    help='The number of workers devoted to running this game.')
parser.add_argument('-s', metavar='start-generation', type=int, default=0,
                    help='Specify which generation to start working on. (Default 0)')

args = parser.parse_args()
rom                      = args.G
executable               = args.e
generateExec             = args.p
dataFile                 = args.d
maxGeneration            = args.g
resultsDir               = args.r
individualsPerGeneration = args.n
numWorkers               = args.w
startGen                 = args.s

# Create results directory if it doesnt exist
if not os.path.isdir(resultsDir):
    os.makedirs(resultsDir)

# Make sure the results directory is empty. We don't want to over-write past results.
if startGen == 0 and not os.listdir(resultsDir) == [] and not os.listdir(resultsDir) == ['nohup.out']:
    print 'Results directory is not empty. Please clear it and re-run.'
    sys.exit(0)
    
# Create Generation 0
if startGen == 0:
    gen0File = os.path.join(resultsDir,"generation0.xml")
    subprocess.call(["./" + generateExec, "-I", dataFile, "-O", gen0File, "-G", rom])
else:
    # Make sure there is a generation file for the specified start generation
    genPath = os.path.join(resultsDir,"generation"+str(startGen)+".xml.gz")
    if not os.path.exists(genPath):
        print 'Generation File:',genPath,'for requested starting generation',startGen,'does not exist...'
        sys.exit(0)

# Start worker threads running
procIDs = [] # Keep track of the ids of the condor jobs
for i in range(numWorkers):
    pid = startWorker(i, executable, resultsDir, dataFile, individualsPerGeneration, maxGeneration, rom)
    procIDs.append(pid)

currentGeneration = startGen
while currentGeneration < maxGeneration:
    individualIds = range(individualsPerGeneration)
    while individualIds:
        for individualId in individualIds:
            fitnessFile = "fitness."+str(currentGeneration)+"."+str(individualId)
            fitnessPath = os.path.join(resultsDir,fitnessFile)

            if os.path.exists(fitnessPath):
                # If the file has been written to -- remove from list
                if os.path.getsize(fitnessPath) > 0:
                    individualIds.remove(individualId)
                # Delete fitness file if it hasn't been written to in 30 mins. Job is likely dead or fubar.
                else if time.time() - os.path.getmtime(fitnessPath) > 1800:
                    print 'Deleting empty and old fitness file',fitnessPath
                    os.remove(fitnessPath)

        # Restart any workers that have ended unexpectedly
        out = subprocess.Popen(["condor_q"], stdout=subprocess.PIPE).communicate()[0]
        for procID in procIDs:
            if out.find(procID+'   mhauskn') == -1:
                indx = procIDs.index(procID)
                procIDs.remove(procID)
                pid = startWorker(indx, executable, resultsDir, dataFile, individualsPerGeneration,
                                  maxGeneration, rom)
                procIDs.append(pid)
                print 'Process',procID,'dissapeared... Starting a new worker.'
                
        # Wait for a little while 
        time.sleep(1)

    # Create next generation
    currGenFile = os.path.join(resultsDir,"generation"+str(currentGeneration)+".xml.gz")
    nextGenFile = os.path.join(resultsDir,"generation"+str(currentGeneration+1)+".xml")
    evalGenFile = os.path.join(resultsDir,"generation"+str(currentGeneration)+".eval.xml")
    fitnessRoot = os.path.join(resultsDir,"fitness." + str(currentGeneration)+".")
    subprocess.Popen(["./" + generateExec, "-I", dataFile, "-O", nextGenFile, "-P", currGenFile,
                     "-F", fitnessRoot, "-E", evalGenFile, "-G", rom])

    currentGeneration += 1


