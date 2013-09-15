#                . '@(@@@@@@@)@. (@@) `  .   '
#      .  @@'((@@@@@@@@@@@)@@@@@)@@@@@@@)@ 
#      @@(@@@@@@@@@@))@@@@@@@@@@@@@@@@)@@` .
#   @.((@@@@@@@)(@@@@@@@@@@@@@@))@\@@@@@@@@@)@@@  .
#  (@@@@@@@@@@@@@@@@@@)@@@@@@@@@@@\\@@)@@@@@@@@)
# (@@@@@@@@)@@@@@@@@@@@@@(@@@@@@@@//@@@@@@@@@) ` 
#  .@(@@@@)##&&&&&(@@@@@@@@)::_=(@\\@@@@)@@ .   .'
#    @@`(@@)###&&&&&!!;;;;;;::-_=@@\\@)@`@.
#    `   @@(@###&&&&!!;;;;;::-=_=@.@\\@@     '
#       `  @.#####&&&!!;;;::=-_= .@  \\
#             ####&&&!!;;::=_-        `
#              ###&&!!;;:-_=
#               ##&&!;::_=
#              ##&&!;:=
#             ##&&!:-
#            #&!;:-
#           #&!;=
#           #&!-
#            #&=
#    jgs      #&-
#             \\#/'
#  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
# The generator is in charge of generating new populations at each generation

import argparse, os, random, time, sys, util, subprocess, atexit

parser = argparse.ArgumentParser(description='Creates new generations when required.')
parser.add_argument('-e', metavar='atari_generate', required=True,
                    help='This should point to atari_generate executable.')
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
parser.add_argument('-R', metavar='random-seed', required=False, type=int, default=-1,
                    help='Seed the random number generator.')

args = parser.parse_args()
rom                      = args.G
seed                     = str(args.R)
executable               = args.e
dataFile                 = args.d
maxGeneration            = args.g
resultsDir               = args.r
individualsPerGeneration = args.n

# TODO: Check if there exists a .bak file
for f in os.listdir(resultsDir):
    if f.startswith('generation') and (f.endswith('.bak') or f.endswith('.tmp')):
        subprocess.check_call(["mv", f, f[0:-4]])

currentGeneration = util.getCurrentGen(resultsDir)

# Generate the first generation if not present
if currentGeneration < 0:
        currentGeneration = 0
        os.system("echo Starting Generation Production... >> " + os.path.join(resultsDir,"nohup.out"))
        os.system("date >> " + os.path.join(resultsDir,"nohup.out"))

        cmd = "./" + executable + \
        " -R " + str(seed) + \
        " -I " + dataFile + \
        " -O " + os.path.join(resultsDir,"generation0.ser.gz") + \
        " -G " + rom + " >> " + os.path.join(resultsDir,"nohup.out")
        os.system(cmd)
        # subprocess.check_call(["./" + executable,
        #                        "-R", str(seed),
        #                        "-I", dataFile,
        #                        "-O", os.path.join(resultsDir,"generation0.ser.gz"),
        #                        "-G", rom])
        os.system("date >> " + os.path.join(resultsDir,"nohup.out"))
        os.system("echo Generation Production Done. >> " + os.path.join(resultsDir,"nohup.out"))

while currentGeneration < maxGeneration:
    print 'Starting Generation', currentGeneration

    # Loop until all fitness files for current generation are present
    individualIds = range(individualsPerGeneration)
    while individualIds:
        for individualId in list(individualIds):
            fitnessFile = "fitness."+str(currentGeneration)+"."+str(individualId)
            fitnessPath = os.path.join(resultsDir,fitnessFile)
            if os.path.exists(fitnessPath):
                individualIds.remove(individualId)
        time.sleep(5)

    # Create next generation
    currGenFile = os.path.join(resultsDir,"generation"+str(currentGeneration)+".ser.gz")
    nextGenFile = os.path.join(resultsDir,"generation"+str(currentGeneration+1)+".ser.gz")
    tmpCurrGen  = currGenFile + ".bak"
    tmpNextGen  = nextGenFile + ".tmp"

    # Current gen gets moved to tmp file
    subprocess.check_call(["mv", currGenFile, tmpCurrGen])

    os.system("echo Starting Generation Production... >> " + os.path.join(resultsDir,"nohup.out"))
    os.system("date >> " + os.path.join(resultsDir,"nohup.out"))
    # Create the actual next generation. This may take a while...
    fitnessRoot = os.path.join(resultsDir,"fitness." + str(currentGeneration)+".")
    cmd = "./" + executable + \
    " -I " + dataFile + \
    " -R " + str(seed) + \
    " -O " + tmpNextGen + \
    " -P " + tmpCurrGen + \
    " -F " + fitnessRoot + \
    " -G " + rom + " >> " + os.path.join(resultsDir,"nohup.out")
    os.system(cmd)
    os.system("date >> " + os.path.join(resultsDir,"nohup.out"))
    os.system("echo Generation Production Done. >> " + os.path.join(resultsDir,"nohup.out"))

    # subprocess.check_call(["./" + executable,
    #                        "-I", dataFile,
    #                        "-R", str(seed),
    #                        "-O", tmpNextGen,
    #                        "-P", tmpCurrGen,
    #                        "-F", fitnessRoot,
    #                        "-G", rom])

    # Move the temp next gen file to the actual one.
    # This is necessary to keep workers from trying to read the
    # next gen file as it was being written.
    subprocess.check_call(["mv", tmpNextGen, nextGenFile])

    # Delete current generation file
    os.remove(tmpCurrGen)

    currentGeneration += 1
