/* *****************************************************************************
 *  random_tools.h
 *  
 *  Implementation of a set of tools for random number generation
 *
 **************************************************************************** */
#ifndef RANDOM_TOOLS_H
#define RANDOM_TOOLS_H

#include <vector>
#include <cstdlib> 
#include "bspf.hxx"
#include <fstream>
#include <sstream>
#include <iterator>
#include <limits>

/* *********************************************************************
    Returns a random integer within the [lowest, highest] range.
    Code taken from here: http://www.daniweb.com/forums/thread1769.html
 ******************************************************************** */
inline int rand_range(int lowest, int highest) {
    int range = highest - lowest + 1;
    return (rand() % range) + lowest; 
}

/* *********************************************************************
    Returns a random element of the given vector
 ******************************************************************** */
template <class T> 
inline T choice(const vector<T>* p_vec) {
	assert(p_vec->size() > 0);
    unsigned int index = rand_range(0, p_vec->size() - 1);
    assert(index >= 0);
    assert(index < p_vec->size());
    return (*p_vec)[index];
}



#endif
