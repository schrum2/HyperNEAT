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
    call([executable, "-I", dataFile, "-P", generationFile, "-N",
                     str(individualId), "-F", fitnessFile, "-G", rom])
                    
parser = argparse.ArgumentParser(description='Runs Atari games without tire.')
parser.add_argument('-e', metavar='executable-file', help='This should point to atari_evaluate executable.')
parser.add_argument('-r', metavar='results-dir',
                    help='This should point to the directory that results are being accumulated in.')
parser.add_argument('-d', metavar='data-file', help='This should point to the AtariExperiment.dat file')
parser.add_argument('-n', metavar='num-individuals', help='This is the number of individuals in each generation.',
                    type=int)
parser.add_argument('-g', metavar='num-generations',
                    help='This is the number of generats before the run is complete.',type=int);
parser.add_argument('-G', metavar='rom-file', help='This should point to the rom to be run.')

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
    if os.path.exists(os.path.join(resultsDir,evalFile)):
        currentGeneration += 1
        continue

    generationFile = "generation" + str(currentGeneration) + ".xml.gz"
    generationPath = os.path.join(resultsDir,generationFile)

    # Look for fitness files which indicate that games are being run
    individualIds = range(individualsPerGeneration)
    random.shuffle(individualIds)
    for individualId in individualIds:
        fitnessFile = "fitness"+str(currentGeneration)+"."+str(individualId)
        fitnessPath = os.path.join(resultsDir,fitnessFile)
        if os.path.exists(fitnessPath):
            continue
        open(fitnessPath,'w').close() # Touch this fitness file
        run_game(executable, dataFile, generationPath, individualId, fitnessPath, rom)

    # At this point we just need to wait for the master to create the next generation
    time.sleep(.1)
