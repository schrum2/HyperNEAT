/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  shape_tools.cpp
 *
 *   A number of utility functions to work with 2D shapes
 **************************************************************************** */
#include "shape_tools.h"
#include "vector_matrix_tools.h"

/* *****************************************************************************
	Flips a 2D shape matrix horizentaly or vertically
 **************************************************************************** */
void flip_shape(const IntMatrix* orig_matrix, IntMatrix* new_matrix, 
				bool do_horizental, bool do_vertical) {
	new_matrix->clear();
	int m = orig_matrix->size();
	int n = (*orig_matrix)[0].size();
	int row_start, row_end, row_update;
	int col_start, col_end, col_update;
	if (do_horizental) {
		col_start = n - 1;
		col_end = 0;
		col_update = -1;
	} else {
		col_start = 0;
		col_end = n - 1;
		col_update = 1;	
	}
	if (do_vertical) {
		row_start = m - 1;
		row_end = 0;
		row_update = -1;
	} else {
		row_start = 0;
		row_end = m - 1;
		row_update = 1;	
	}
	int i = row_start;
	while(true) {		// row loop
		IntVect row;
		int j = col_start;
		while (true) {	// column loop
			row.push_back((*orig_matrix)[i][j]);
			if (j == col_end) {
				break;
			}
			j = j + col_update;
		}
		new_matrix->push_back(row);
		if (i == row_end) {
			break;
		}
		i = i + row_update;
	}
}

/* *****************************************************************************
	Calculates the pixel and percentage distance between two shape matrices.
	objects. Returns true if the distance is computable (i.e. the 
	objects have compatible shapes).
 **************************************************************************** */
bool calc_shape_distance(	const IntMatrix* pm_shape_a, const IntMatrix* pm_shape_b, 
					bool allow_different_size, float max_shape_area_dif,
					int& pixel_distance, float& perc_distance) {
	int a_height = pm_shape_a->size();
	int a_width  = (*pm_shape_a)[0].size();
	int b_height = pm_shape_b->size();
	int b_width  = (*pm_shape_b)[0].size();
	
	int a_area = a_width * a_height;
	int b_area = b_width * b_height;
	if (a_width == b_width && 
		a_height == b_height) {
		// Objects have the same shape
		pixel_distance = get_l2_distance(pm_shape_a, pm_shape_b);
	} else {
		// Objects have different shapes
		if (!allow_different_size) {
			pixel_distance = -1;
			perc_distance = -1.0;
			return false;
		}
		
		// When one shape is smaller than the other, we move the smaller
		// object inside the bigger object, and return the smallest distance
		// This only makes sense when the shapes area is within a small range
		if  (	( (float)a_area >  (max_shape_area_dif * b_area) ) || 
				( (float)b_area > (max_shape_area_dif * a_area) )	) {
			// size difference is too big
			pixel_distance = -1;
			perc_distance = -1.0;
			return false;
		}

		const IntMatrix *bigger_shape, *smaller_shape;
		int width_dif, height_dif, smaller_height, smaller_width;
		if (a_width <= b_width && 
			a_height <= b_height) {
			bigger_shape = pm_shape_b;
			smaller_shape = pm_shape_a;
			smaller_height = a_height; 
			smaller_width = a_width;
			width_dif = b_width - a_width;
			height_dif = b_height - a_height;
		} else if (	a_width >= b_width && 
					a_height >= b_height) {
			bigger_shape = pm_shape_a;
			smaller_shape = pm_shape_b;
			smaller_height = b_height; 
			smaller_width = b_width;
			width_dif = a_width - b_width;
			height_dif =  a_height - b_height;
		} else {
			// incompatible shapes
			pixel_distance = -1;
			perc_distance = -1.0;
			return false;
		}
		
		assert (width_dif >= 0 &&  height_dif >= 0);
		// TODO: if deemed enccesary, bring this back:
		// if shape(smaller_shape) < (3,3):
		//	# This method (hack) doesn't really make sense for tiny shapes
		//	return None 
		
		int smallest_distance = -1;
		for (int i = 0; i <= height_dif; i++) {
			for (int j = 0; j <= width_dif; j++) {
				int l2_dist = 0;
				for (int y = 0; y < smaller_height; y++) {
					for (int x = 0; x < smaller_width; x++) {
						int dif =	(*bigger_shape)[i+y][j+x] - 
									(*smaller_shape)[y][x];
						l2_dist += dif * dif;
					}
				}
				if (smallest_distance == -1 || l2_dist < smallest_distance) {
					smallest_distance = l2_dist;
				}
			}
		}
		pixel_distance = smallest_distance;
	}
	int num_pixels = a_area + b_area;
	perc_distance = float(pixel_distance) / float(num_pixels);
	return true;
}

 