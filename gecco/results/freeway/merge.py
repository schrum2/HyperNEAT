#!/usr/bin/python

import csv
import numpy
import matplotlib.pyplot as plt

numFiles = 10
numGenerations = 250
errorbarStep = 25
filenames = ['champion','average']
legend = ['Champion Fitness', 'Average Fitness']
colors = ['r', 'b']
linewidth = 2

plt.figure()
for file_number in range(len(filenames)):

  filename = filenames[file_number]
  color = colors[file_number]
  legend_val = legend[file_number]

  generations = []
  means = []
  stddev = []

  generations_error = []
  means_error = []
  stddev_error = []

  #values = range(1,numGenerations+1)
  values = []
  for row in range(numGenerations):
    #values[row] = [values[row]]
    values.append([])

  for i in range(numFiles):
    new_file = csv.reader(open(str(i) + '-' + filename + '.csv', 'rb'))
    new_file = list(new_file)
    for row in range(numGenerations):
      values[row].append(float(new_file[row][0]) - 10)

  file_writer = csv.writer(open(filename + '.csv', 'wb'))
  for row in range(numGenerations):
    #calculate data for plotting
    generations.append(row+1)
    means.append(numpy.mean(values[row]))
    stddev.append(numpy.std(values[row]))
    if ((row+1)%errorbarStep == 0):
      generations_error.append(row+1)
      means_error.append(numpy.mean(values[row]))
      stddev_error.append(numpy.std(values[row]))

    mean_val = numpy.mean(values[row])
    #print csv to file with mean and generation ready for chart creation
    values[row].append(row+1)
    values[row].append(mean_val)
    file_writer.writerow(values[row])
    #print [row+1, numpy.mean(values[row]), numpy.std(values[row])]

  plt.errorbar(generations_error,means_error,yerr=stddev_error,fmt=None,ecolor=color,label='_nolegend_',linewidth=linewidth,capsize=2*linewidth,mew=linewidth)
  plt.plot(generations, means, color+'-', label=legend_val, linewidth=linewidth)
plt.legend()
plt.xlim(0,255)
plt.ylim(0,35)
#plt.show()
plt.grid()
plt.xlabel('Number of generations')
plt.ylabel('Fitness')
plt.savefig('figure.png', bbox_inches='tight', dpi=200)
