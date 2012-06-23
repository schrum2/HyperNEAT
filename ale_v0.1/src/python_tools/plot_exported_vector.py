"""
##############################################################################
# plot_exported_vector.py
#
# Plots and saves a vector that has been exported to file using export_vector
# function
##############################################################################
"""
import  sys
from    sys     import  exit   
from    pylab   import  *
from    scipy   import  *
from    os      import  system

def plot_exported_vector(exported_file, delimiter = ',', num_steps = 1):
    """
        Plots and saves a vector that has been exported to file using
        export_vector function
        num_steps determines the walking average steps.
    """
    fin = open(exported_file)
    inline = fin.readline()
    fin.close()
    inline_split = inline.split(delimiter)
    x = []
    for val in inline_split:
        if val.strip() != "":
            x.append(float(val))
    walking_avg_x = get_moving_average(x, num_steps)
    plot(walking_avg_x)
    filname = exported_file[0:exported_file.rfind('.')]
    if num_steps != 1:
        filname += "_ma_%d_steps"%num_steps
    filname += ".png"
    savefig(filname)


def get_moving_average(vec, num_steps):
    if num_steps == 1:
        return vec
    moving_average_vec = []
    for i in range(num_steps, len(vec) - num_steps):
        sum = 0
        sum += vec[i]
        for j in range(1, num_steps + 1):
            sum += vec[i - j]
            sum += vec[i + j]
        average = float(sum) / float(2 * num_steps + 1)
        moving_average_vec.append(average)
    return moving_average_vec
    
if __name__ == "__main__":
    arg_len = len(sys.argv)
    if arg_len < 2 or arg_len > 4:
        print "Usage: "
        print "\t python plot_exported_vector.py exported_file " + \
                "[Moving Average Steps] [delimiter] "
                
        exit(-1)
    delimiter = ","
    num_steps = 1
    if arg_len == 3:
        num_steps = int(sys.argv[2])
    if arg_len == 4:
        num_steps = int(sys.argv[2])
        delimiter = sys.argv[3]
    vector_file = sys.argv[1]
    is_compressed = False
    if vector_file.find('.gz') > 0:
        system("gunzip %s" % vector_file)
        vector_file = vector_file[:vector_file.rfind('.gz')]
        is_compressed = True
    plot_exported_vector(vector_file, delimiter, num_steps)
    if is_compressed:
        system("gzip %s" % vector_file)

