#!/usr/bin/python

import sys

if len(sys.argv) < 2:
    print 'Usage:',sys.argv[0],'nohup.out'
    sys.exit(0)

avgFit, champFit = [],[]
f = open(sys.argv[1],'r')
for line in f:
    if str.startswith(line,'Champion fitness'):
        champFit.append(int(line.split()[2]))
    elif 'overall_average' in line:
        avgFit.append(float(line.split()[4]))
f.close()

import matplotlib.pyplot as plt

x1 = range(len(champFit))
x2 = range(len(avgFit))
plt.plot(x1,champFit,x2,avgFit)
plt.xlabel('Generation')
plt.ylabel('Fitness')
plt.show()
