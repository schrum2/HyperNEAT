/*
 *  vector_matrix_tools.cpp
 *
 *
 */

#include "vector_matrix_tools.h"

/* *****************************************************************************
    Normalizes the values in a matrix to fit within [0, 1]
 **************************************************************************** */
 void normalize_matrix(FloatMatrix* pm_matrix) {
	double min_val = (*pm_matrix)[0][0];
	double max_val = (*pm_matrix)[0][0];
	for (FloatMatrix::iterator it_row = pm_matrix->begin(); 
		 it_row != pm_matrix->end(); ++it_row) {
		for (FloatVect::iterator it_col = (*it_row).begin();
			 it_col != (*it_row).end(); ++it_col) {
			double value = (*it_col);
			if (value == 1313.0 ||	// TMP HACK
				value == 1314.0) {
				continue;
			}
			if (value < min_val) {
				min_val = value;
			}
			if (value > max_val) {
				max_val = value;
			}
		}
	}
	double range = max_val - min_val;
	assert (range >= 0.0);
	if (range == 0.0) {
		return; // nothing to do
	}
	// normalize the matrix
	for (FloatMatrix::iterator it_row = pm_matrix->begin(); 
		 it_row != pm_matrix->end(); ++it_row) {
		for (FloatVect::iterator it_col = (*it_row).begin();
			 it_col != (*it_row).end(); ++it_col) {
			if ((*it_col) != 1313.0 &&	// TMP HACK
				(*it_col) != 1314.0) {
				(*it_col) = (*it_col) - min_val;
				(*it_col) = (*it_col) / range;
			}
		}
	}	
 }
 
 /* *****************************************************************************
     A simpel version of 'where' function in Matlab/SciPy
	Fills pv_row_inds and pv_col_inds, with all i, j such that: M[i,j] == target
 **************************************************************************** */
void simple_where(	const IntMatrix* pm_matrix, int target, 
					IntVect* pv_row_inds, IntVect* pv_col_inds) {
	pv_row_inds->clear();
	pv_col_inds->clear();
	int height = pm_matrix->size();
	if (height == 0) {
		return;	// empty matrix, nothing to do
	}
	int width = (*pm_matrix)[0].size();
	if (width == 0) {
		return;	// empty matrix, nothing to do
	}
	for (int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			if ( (*pm_matrix)[i][j] == target ) {
				pv_row_inds->push_back(i);
				pv_col_inds->push_back(j);
			}
		}
	}
}

/* *****************************************************************************
    Shrinks any value less smallest_val than in the vector to zero
 **************************************************************************** */
void shrink_array(FloatArr* pv_vector, float smallest_val) {
	if (smallest_val == 0) {
		cout << "smallest_val = 0. Cannot shrink the vector" << endl;
		return;
	}
	assert (smallest_val > 0);
	for (unsigned int i = 0; i < pv_vector->size(); i++) {
		if (abs((*pv_vector)[i]) < smallest_val) {
			(*pv_vector)[i] = 0;
		}
	}
}

/* *****************************************************************************
    Shrinks the evctor by keeping num_values_to_keep highest values, and 
	converting all other values to zero
 **************************************************************************** */
void shrink_array(FloatArr* pv_vector, int num_values_to_keep) {
	FloatVect* sorted_copy = new FloatVect;
	for (unsigned int i = 0; i < pv_vector->size(); i++) {
		sorted_copy->push_back(abs((*pv_vector)[i]));
	}
	sort(sorted_copy->begin(), sorted_copy->end());
	reverse(sorted_copy->begin(), sorted_copy->end());
	float smallest_val = (*sorted_copy)[num_values_to_keep - 1];
	shrink_array(pv_vector, smallest_val);
	delete sorted_copy;
}