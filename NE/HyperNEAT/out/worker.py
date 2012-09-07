# worker.py ----------------------
#              ___
#            ."   ".
#            |  ___(
#            ).' -(
#             )  _/
#           .'_`( 
#          / ( ,/;
#         /   \ ) \\.
#        /'-./ \ '.\\)
#        \   \  '---;\
#        |`\  \      \\
#       / / \  \      \\
#     _/ /   / /      _\\/
#    ( \/   /_/       \   |
#     \_)  (___)       '._/
#--------------------------------

import argparse

parser = argparse.ArgumentParser(description='Runs Atari games without tire.')
parser.add_argument('-i', metavar='in-file', type=argparse.FileType('r'))
parser.add_argument('-o', metavar='out-file', type=argparse.FileType('w'))
parser.add_argument('-e', metavar='executable-file', type=argparse.FileType('r'))
try:
    results = parser.parse_args()
    print 'Input file:', results.i
    print 'Output file:', results.o
    print 'Executable file:', results.e
except IOError, msg:
    parser.error(str(msg))

