#!/usr/bin/python

import random

for i in range(100):
    f = open('results/fitness.0.'+str(i),'w')
    fit = int(random.random() * 100 - 50)
    f.write(str(fit))
    f.close()
