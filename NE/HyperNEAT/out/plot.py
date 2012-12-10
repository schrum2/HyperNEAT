#!/usr/bin/python

import sys

colors = ['b','g','r','c','m','y','k','w']

if len(sys.argv) < 2:
    print 'Usage:',sys.argv[0],'nohup.out'
    sys.exit(0)

for i in xrange(1,len(sys.argv)):
    avgFit, champFit, stdErr, stdDev = [],[],[],[]
    f = open(sys.argv[i],'r')
    for line in f:
        if str.startswith(line,'Champion fitness'):
            champFit.append(int(line.split()[2]))
        elif 'overall_average' in line:
            avgFit.append(float(line.split()[4]))
        elif 'StandardError' in line:
            stdErr.append(float(line.split()[-1]))
            stdDev.append(float(line.split()[-3]))
    f.close()

    import matplotlib.pyplot as plt

    x1 = range(len(champFit))
    x2 = range(len(avgFit))

    c = colors[i-1]
    plt.plot(x1,champFit,x2,avgFit,color=c)
    plt.plot(x2,[avgFit[i] + stdDev[i] for i in x2],':',color=c)
    plt.plot(x2,[avgFit[i] - stdDev[i] for i in x2],':',color=c)

plt.xlabel('Generation')
plt.ylabel('Fitness')
plt.show()
