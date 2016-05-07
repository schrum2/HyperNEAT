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
# The master is in charge of monitoring all the condor jobs
# to make sure the generator and workers continue onwards
import argparse, os, subprocess, time, sys, util

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
parser.add_argument('-R', metavar='random-seed', required=False, type=int, default=-1,
                    help='Seed the random number generator.')

args = parser.parse_args()
rom                      = args.G
seed                     = args.R
executable               = args.e
generateExec             = args.p
dataFile                 = args.d
maxGeneration            = args.g
resultsDir               = args.r
individualsPerGeneration = args.n
numWorkers               = args.w
doingCMAES = dataFile.endswith('AtariCMAExperiment.dat')

if not os.path.exists(rom):
    print 'Rom not found. Exiting.'
    sys.exit(0)

# Create results directory if it doesnt exist
if not os.path.isdir(resultsDir):
    os.makedirs(resultsDir)

workerNum = 0
genNum = 0
deadWorkers = 0
deadWorkerLimit = 999999999
procIDs = {} # Maps pid --> worker num
genPID = util.startGenerator(genNum, generateExec, resultsDir, dataFile, individualsPerGeneration,
                        maxGeneration, seed, rom)
genNum += 1

# Main Loop
while util.getCurrentGen(resultsDir,doingCMAES) < maxGeneration:
    time.sleep(10)

    out = util.condor_q()
    if not out:
        continue

    # print 'condor_q output',out
    # sys.stdout.flush()
        
    # Check if generator is alive & active
    genStatus = util.getPIDStatus(genPID, out) 
    if genStatus == None: # Missing generator
        print 'Missing Generator. pid:',genPID
        sys.exit(0)
    elif genStatus != 'R': # Generator alive but not running
        continue

    # Write file to tell generator that master is still alive
    open(os.path.join(resultsDir,'master_alive'),'w').close()

    # Don't start workers if generation is being produced
    if util.getCurrentGen(resultsDir,doingCMAES) < 0:
        continue

    # Detect missing workers
    for procID in list(procIDs):
        if out.find('\n'+str(procID)) == -1:
            # print 'Missing pid:',procID
            # sys.stdout.flush()

            indx = procIDs[procID]
            del procIDs[procID]

            # Read its error log and save to global error log
            glog = open(os.path.join(resultsDir,'global.err'),'a')
            errFile = os.path.join(resultsDir,'worker'+str(indx)+'.err')
            if os.path.exists(errFile) and os.path.getsize(errFile) > 0:
                deadWorkers += 1
                llog = open(errFile,'r')
                glog.write('Worker number '+str(indx)+' pid '+str(procID)+' died with the following error log:\n')
                glog.write(llog.read())
                llog.close()
            glog.close()

            # Remove associated worker files
            if os.path.exists(os.path.join(resultsDir,'worker'+str(indx)+'.err')):
                os.remove(os.path.join(resultsDir,'worker'+str(indx)+'.err'))
            if os.path.exists(os.path.join(resultsDir,'worker'+str(indx)+'.log')):
                os.remove(os.path.join(resultsDir,'worker'+str(indx)+'.log'))
            if os.path.exists(os.path.join(resultsDir,'worker'+str(indx)+'.submit')):
                os.remove(os.path.join(resultsDir,'worker'+str(indx)+'.submit'))
            if os.path.exists(os.path.join(resultsDir,'worker'+str(indx)+'.out')):
                os.remove(os.path.join(resultsDir,'worker'+str(indx)+'.out'))      

        # Stop the master if too many workers have died
        if deadWorkers >= deadWorkerLimit:
            readBody = subprocess.Popen(["echo", resultsDir], stdout=subprocess.PIPE)
            subprocess.check_call(["mail", "-s", '[master.py - '+resultsDir+'] too many workers dead', 'schrum2@cs.utexas.edu'], stdin=readBody.stdout, stdout=subprocess.PIPE)
            print 'Over',deadWorkerLimit,'dead workers. When will the violence end?'
            sys.stdout.flush()
            sys.exit(0)

    # Start a worker
    if len(procIDs) < numWorkers:
        pid = util.startWorker(workerNum, executable, resultsDir, dataFile,
                          individualsPerGeneration, maxGeneration, seed, rom)
        if pid > 0:
            # print 'Started worker',workerNum,'pid:',pid
            # sys.stdout.flush()
            procIDs[pid] = workerNum
            workerNum += 1





