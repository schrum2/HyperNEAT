"""
##############################################################################
# avg_exported-vector.py
#
# Gets the average for thw whole or teh final sectin of a  vector that has been
# exported to file using export_vector function
##############################################################################
"""
import  sys
from    sys     import  exit   
from    scipy   import  *
from    os      import  system

def avg_exported_vector(exported_file, end_part_length = None):
    fin = open(exported_file)
    inline = fin.readline()
    fin.close()
    inline_split = inline.split(",")
    x = []
    for val in inline_split:
        if val.strip() != "":
            x.append(float(val))
    v = array(x)
    v_len = len(x)
    if end_part_length is None:
        end_part_length = v_len
    assert end_part_length <= v_len
    v_end_part = v[v_len - end_part_length:v_len]
    avg = float(sum(v_end_part)) / float(end_part_length)
    return avg

if __name__ == "__main__":
    arg_len = len(sys.argv)
    if arg_len < 2 or arg_len > 3:
        print "Usage: "
        print "\t python avg_exported_vector.py exported_file " + \
                "[Length of the end part of the vector] "
                
        exit(-1)
    end_part_length = None
    if arg_len == 3:
        end_part_length = int(sys.argv[2])
    is_compressed = False
    vector_file = sys.argv[1]
    if vector_file.find('.gz') > 0:
        system("gunzip %s" % vector_file)
        vector_file = vector_file[:vector_file.rfind('.gz')]
        is_compressed = True
    avg = avg_exported_vector(vector_file, end_part_length)
    print "Average = ", avg

    if is_compressed:
        system("gzip %s" % vector_file)

