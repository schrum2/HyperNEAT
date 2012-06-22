/* *****************************************************************************
 *  misc_tools.h
 *
 *  A set of miscalenous tools used in various places.
 **************************************************************************** */
#ifndef MISC_TOOLS_H
#define MISC_TOOLS_H

#include "../player_agents/common_constants.h"

/* *****************************************************************************
    Inline C++ integer exponentiation routines 
    Version 1.01
    Copyright (C) 1999-2004 John C. Bowman <bowman@math.ualberta.ca>
 **************************************************************************** */
inline int pow(int x, int p) {
    if(p == 0) return 1;
    if(x == 0 && p > 0) return 0;
    if(p < 0) {assert(x == 1 || x == -1); return (-p % 2) ? x : 1;}

    int r = 1;
    for(;;) {
    if(p & 1) r *= x;
    if((p >>= 1) == 0)	return r;
    x *= x;
    }
}

/* *****************************************************************************
    Makes x fit within the [uper, lower] bounds
 **************************************************************************** */
inline void bound(int& x, int lower_bound, int upper_bound) {
    if (x > upper_bound) {
        x = upper_bound;
    }
    if (x < lower_bound) {
        x = lower_bound;
    }
}


#endif
