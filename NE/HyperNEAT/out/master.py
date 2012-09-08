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

import argparse, os, subprocess

# Submits a condor job which starts a worker running
def startWorker():
    print 'Submitting Condor Job'

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

args = parser.parse_args()
rom                      = args.G
executable               = args.e
generateExec             = args.p
dataFile                 = args.d
maxGeneration            = args.g
resultsDir               = args.r
individualsPerGeneration = args.n
numWorkers               = args.w

# Create results directory if it doesnt exist
if not os.path.isdir(resultsDir):
    os.makedirs(resultsDir)

# Create Generation 0
gen0File = os.path.join(resultsDir,"generation0.xml")
subprocess.call(["./" + generateExec, "-I", dataFile, "-O", gen0File, "-G", rom])

# Start worker threads running
for i in range(numWorkers):
    cOutFile = os.path.join(resultsDir,"worker" + str(i) + ".out")
    cErrFile = os.path.join(resultsDir,"worker" + str(i) + ".err")
    cLogFile = os.path.join(resultsDir,"worker" + str(i) + ".log")
    confStr = "\
    Output = " + cOutFile +"\n\
    Error = " + cErrFile +"\n\
    Log = " + cLogFile + "\n\
    universe = vanilla\n\
    Executable = " + "ls" + "\n\
    Arguments = -la\n\
    Requirements = Lucid && (Arch == \"x86_64\" || Arch==\"INTEL\") && (1024 * Memory >= 10)\n\
    +Group=\"GRAD\"\n\
    +Project=\"AI_ROBOTICS\"\n\
    +ProjectDescription=\"HyperNEAT Atari Game Playing.\"\n\
    Queue\n"
    subprocess.call(["condor_submit","-verbose",confStr])
