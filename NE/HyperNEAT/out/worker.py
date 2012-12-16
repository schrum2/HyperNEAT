#                     >,o~o\/)-'     ,-'         \       ,.__.,-
#                     <\__,'c~      (.---..____..,'      >'oo \~)
#                        |--'_.---~~~~:     __.......__ `-`---'(
#                       ',-~~         |   /'           `~~-. `'       .-~~~~.
#                      ','            ; ,'                  |    '~~\  |     `.
#                    ,','            ; ;                    |  ,'    | |      |
#                  ,.|,'            / |                     |  |     | `.     |
#                 | \`|           ,'  |                     | _.--,  `. |     `.
#                 |  | \      _.-'___                   _.--~~,-',    | |      |
#                 |  `. |--__..-~~   ~~~~~---...___.--~~ _,-'~ ,|     | `.__,--'
#                 |   | /';                     ./'   _,'     / |_,--~~
#                 `   /' |                _.---'  _,-'      ,'
#                  \/'   |              /'      ~~ ~~--._   ;
#        `\ `\    ,','~~\|            /'   __        .--'  /__,   __
#          `\ `\_.||  ;  ||         /'  ,-~  `~.    '----~~~_.--~~
#            `\/`  |  |  ;______  ,'   |  ,'    |       _.-~  ;               _,
#       --..__|    `\_`,'\     _`\     `. |     ;   .-~~   __..----~~~~~      \
#        _,-~ |  `-       `--~~          `----~'_..-    ~~~  /                |
#      /'--.   \                           _       / ,'  ~~~7~~~~~~~~         |/
#    ,'     ~-. `--.------------....__   /'_\_...-'       ,'`-.      /        |-
# ,-~~~~-._   /   (`-\___|___;...--'--~~~~~            ,-'     |    |         |~
# /~~~-    ~~~     `\                            _,__,'   ___,-'    |         |
# `-.                `-..___...__            _.-'/    ~~~~          ;         ;
# `--`\____       __..---~~ ~~--..~--------~~   |                  ,'       ,'
#                                           "Catbus" (from "My Neighbor Totoro")

import argparse, os, random, time, sys

# This runs a single Atari game.
def run_game(executable, dataFile, generationFile, individualId, fitnessFile, seed, rom):
    from subprocess import check_call
    check_call(["./" + executable, "-I", dataFile, "-P", generationFile, "-N",
                     str(individualId), "-F", fitnessFile, "-R", seed, "-G", rom])
                    
parser = argparse.ArgumentParser(description='Runs Atari games without tire.')
parser.add_argument('-e', metavar='atari_evaulate', required=True,
                    help='This should point to atari_evaluate executable.')
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

# Detect the current generation
currentGeneration = -1
for f in os.listdir(resultsDir):
    if f.startswith('generation') and 'tmp' not in f:
        genNumber = int(f[len('generation'):-len('.ser.gz')])
        currentGeneration = max(currentGeneration, genNumber)
if currentGeneration < 0:
        sys.stderr.write('Did not find any generation files! Exiting.\n')
        sys.stderr.flush()
        sys.exit(0)

while currentGeneration < maxGeneration:
    # Wait until we see a generation file for the current generation
    generationFile = "generation" + str(currentGeneration) + ".ser.gz"
    generationPath = os.path.join(resultsDir,generationFile)

    start = time.time()
    while not os.path.exists(generationPath):
        time.sleep(5)
        if time.time() - start >= 300:
            sys.stderr.write('Reached 5min timeout waiting for new generation... quitting\n')
            sys.stderr.flush()
            sys.exit(0)

    # Look for fitness files which indicate that games are being run
    individualIds = range(individualsPerGeneration)
    random.shuffle(individualIds)
    for individualId in individualIds:
        # Break out of this loop if the generation has ended
        nextGenPath = os.path.join(resultsDir,"generation" + str(currentGeneration+1) + ".ser.gz")
        if os.path.exists(nextGenPath): 
            break
        fitnessFile = "fitness."+str(currentGeneration)+"."+str(individualId)
        fitnessPath = os.path.join(resultsDir,fitnessFile)
        if os.path.exists(fitnessPath):
            continue

        run_game(executable, dataFile, generationPath, individualId, fitnessPath, seed, rom)

    # By this time all fitness evaluations should be complete
    currentGeneration += 1
