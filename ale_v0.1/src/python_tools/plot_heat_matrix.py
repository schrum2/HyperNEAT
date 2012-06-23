"""
##############################################################################
# plot_heat_matrix.py
#
# Plots and saves (as png) a vector that has been exported to file using
# export_vector function
##############################################################################
"""
import  sys
from    sys     import  exit   
from    pylab   import  *
from    scipy   import  *
from    PIL     import Image

def plot_heat_matrix(filename):
    """
        Plots and saves (as png) a heat matrix (values between 0 and 1)
        that has been exported to file  using export_matrix function
        The matrix format
            is as follows:
                width,height\n
                M[0,0], M[0,1], ..., M[0,n]\n
                M[1,0], M[1,1], ..., M[1,n]\n
                ...
                M[m,0], M[m,1], ..., M[m,n]\n    
    """
    # 1- Import the matrix
    delimiter = ","
    fin = open(filename)
    inline = fin.readline()
    inlin_splt = inline.split(delimiter)
    width = int(inlin_splt[0])
    height = int(inlin_splt[1])
    assert width > 0 and height > 0
    rgb_array = zeros((height, width , 3))
    for i in range(height):
        inline = fin.readline()
        inlin_splt = inline.split(delimiter)
        assert (len(inlin_splt) - 1) == width
        for j in range(width):
            new_val = float(inlin_splt[j])
            if new_val == 1313.0:
                r = 255
                g = 0
                b = 0
            elif new_val == 1314.0:
                r = 0
                g = 0
                b = 0
            else:
                assert (new_val >= 0 and new_val <= 1)
                r = g = b = new_val * 255
            rgb_array[i,j,0] = r
            rgb_array[i,j,1] = g
            rgb_array[i,j,2] = b
    fin.close()
    
    # 2- Plot the matrix
    filename = filename[0:filename.rfind('.')]
    filename = filename + ".png"
    print filename
    pilImage = Image.fromarray(rgb_array, 'RGB')
    pilImage.save(filename)

    
    

    
if __name__ == "__main__":
    arg_len = len(sys.argv)
    if arg_len != 2:
        print "Usage: "
        print "\t python plot_heat_matrix.py exported_file "
                
        exit(-1)
    plot_heat_matrix(sys.argv[1])



