/* *****************************************************************************
 *  vector_matrix_tools.h
 *
 *  A set of common vector/matrix tools
 **************************************************************************** */
#ifndef VECTOR_MATRIX_TOOLS_H
#define VECTOR_MATRIX_TOOLS_H

#include "../player_agents/common_constants.h"

/* *****************************************************************************
    Normalizes the values in a matrix to fit within [0, 1]
 **************************************************************************** */
 void normalize_matrix(FloatMatrix* pm_matrix);

 
/* *****************************************************************************
    A simpel version of 'where' function in Matlab/SciPy
	Fills pv_row_inds and pv_col_inds, with all i, j such that: M[i,j] == target
 **************************************************************************** */
 void simple_where(	const IntMatrix* pm_matrix, int target, 
					IntVect* pv_row_inds, IntVect* pv_col_inds); 
 
/* *****************************************************************************
    Shrinks any value less smallest_val than in the valarray to zero
 **************************************************************************** */
 void shrink_array(FloatArr* pv_vector, float smallest_val);

/* *****************************************************************************
    Shrinks the valarray by keeping num_values_to_keep highest values, and 
	converting all other values to zero
 **************************************************************************** */
 void shrink_array(FloatArr* pv_vector, int num_values_to_keep);


/* *****************************************************************************
    Finds the value and the index to the highest value in the vector
 **************************************************************************** */
 template <class T> 
 void vect_max(	const vector<T>* pv_vector, T& max_val, int& max_index) {
	assert (pv_vector->size() > 0);
	max_val = (*pv_vector)[0];
	max_index = 0;
	for (int i = 1; i < pv_vector->size(); i++) {
		T val = (*pv_vector)[i];
		if (val > max_val) {
			max_val = val;
			max_index = i;
		}
	}
 }

/* *****************************************************************************
    Finds the value and the index to the lowest value in the vector
 **************************************************************************** */
 template <class T> 
 void vect_min(	const vector<T>* pv_vector, T& min_val, int& min_index) {
	assert (pv_vector->size() > 0);
	min_val = (*pv_vector)[0];
	min_index = 0;
	for (int i = 1; i < pv_vector->size(); i++) {
		T val = (*pv_vector)[i];
		if (val < min_val) {
			min_val = val;
			min_index = i;
		}
	}
 }
  
 /* *****************************************************************************
    Returns the average value of the given vector V[start:end)
 **************************************************************************** */
 template <class T> 
inline T get_vector_average(const vector<T>* pv_vector, int start, int end) {
	int n = end - start;
	T sum = (T)0.0;
	for (int i = start; i < end; i++) {
		sum += (*pv_vector)[i];
	}
	return sum / (T)n;
}

 /* *****************************************************************************
    Returns the l2-distance of two matrices
 **************************************************************************** */
 template <class T> 
inline T get_l2_distance(	const vector< vector<T> >* pm_matrix_a,
							const vector< vector<T> >* pm_matrix_b) {
	unsigned int m = pm_matrix_a->size();
	unsigned int n = (*pm_matrix_a)[0].size();
	assert ( m == pm_matrix_b->size());
	assert (n == (*pm_matrix_b)[0].size());
	T total_dist = (T)0;
	for (unsigned int i = 0; i < m; i++) {
		for (unsigned int j = 0; j < n; j++) {
			T diff = (*pm_matrix_a)[i][j] - (*pm_matrix_b)[i][j];
			total_dist += diff * diff;
		}
	}
	return total_dist;
}

/* *********************************************************************
	Deletes all the objects inside the list, and replace their pointers 
	with NULL
  ******************************************************************** */
template <class T> 
void clear_list_of_pointers( vector < T* >* pv_vector) {
	for (unsigned int i = 0; i < pv_vector->size(); i++) {
		if ((*pv_vector)[i]) {
			delete (*pv_vector)[i];
			(*pv_vector)[i] = NULL;
		}
	}
}

#endif
