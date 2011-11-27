/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  shape_tools.h
 *
 *   A number of utility functions to work with 2D shapes
 **************************************************************************** */
#ifndef SHAPE_TOOLS_H
#define SHAPE_TOOLS_H

#include "common_constants.h"

/* *****************************************************************************
	Flips a 2D shape matrix horizentaly or vertically
 **************************************************************************** */
void flip_shape(const IntMatrix* orig_matrix, IntMatrix* new_matrix, 
				bool do_horizental, bool do_vertical);

   
/* *****************************************************************************
	Calculates the pixel and percentage distance between two shape matrices.
	objects. Returns true if the distance is computable (i.e. the 
	objects have compatible shapes).
 **************************************************************************** */
bool calc_shape_distance(	const IntMatrix* pm_shape_a, const IntMatrix* pm_shape_b, 
					bool allow_different_size, float max_shape_area_dif,
					int& pixel_distance, float& perc_distance);


 #endif 
 