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

import argparse, os, random, time

# This runs a single Atari game.
def run_game(executable, dataFile, generationFile, individualId, fitnessFile, rom):
    from subprocess import call
    call(["./" + executable, "-I", dataFile, "-P", generationFile, "-N",
                     str(individualId), "-F", fitnessFile, "-G", rom])
                    
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

args = parser.parse_args()
rom                      = args.G
executable               = args.e
dataFile                 = args.d
maxGeneration            = args.g
resultsDir               = args.r
individualsPerGeneration = args.n

currentGeneration = 0
while currentGeneration < maxGeneration:
    # If an eval file for this generation already exists, go to next gen
    evalFile = "generation" + str(currentGeneration) + ".eval.xml.gz"
    evalPath = os.path.join(resultsDir,evalFile)
    if os.path.exists(evalPath):
        currentGeneration += 1
        continue

    # Wait until we see a generation file for the current generation
    generationFile = "generation" + str(currentGeneration) + ".xml.gz"
    generationPath = os.path.join(resultsDir,generationFile)
    while not os.path.exists(generationPath):
        print 'Generation file:',generationFile,'not found... sleeping.'
        time.sleep(1)

    # Look for fitness files which indicate that games are being run
    individualIds = range(individualsPerGeneration)
    random.shuffle(individualIds)
    for individualId in individualIds:
        # Break out of this loop if the generation has ended
        if os.path.exists(evalPath): 
            break
        fitnessFile = "fitness."+str(currentGeneration)+"."+str(individualId)
        fitnessPath = os.path.join(resultsDir,fitnessFile)
        if os.path.exists(fitnessPath):
            continue
        open(fitnessPath,'w').close() # Touch this fitness file
        run_game(executable, dataFile, generationPath, individualId, fitnessPath, rom)

