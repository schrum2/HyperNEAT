/* *****************************************************************************
 *  export_tools.h
 *  
 *  Implementation of a set of tools for exporting various data types
 *
 **************************************************************************** */
#ifndef EXPORT_TOOLS_H
#define EXPORT_TOOLS_H

#include <fstream>
#include <vector>
#include <valarray>
#include <cstdlib> 
#include "bspf.hxx"
#include "common_constants.h"


/* *********************************************************************
    Exports the content of the given vector to file, seperated with
    the delimeter
 ******************************************************************** */
template <class T> 
inline void export_vector(const vector<T>* p_vec, const string& filename,
                            char delimiter = ',') {
    ofstream file;
	file.open(filename.c_str());
	for (unsigned int i = 0; i < p_vec->size(); i++) {
        file << (*p_vec)[i] << delimiter;
    }
    file << endl;
	file.close();
}

/* *********************************************************************
    Exports the content of the given array to file, seperated with
    the delimeter
 ******************************************************************** */
 template <class T> 
inline void export_array(const valarray<T>* p_arr, const string& filename,
                            char delimiter = ',') {
    ofstream file;
	file.open(filename.c_str());
	for (unsigned int i = 0; i < p_arr->size(); i++) {
        file << (*p_arr)[i] << delimiter;
    }
    file << endl;
	file.close();
}

/* *********************************************************************
    Imports the content of the given vector from file, seperated with
    Note: previous content of the vecor will be cleared.
 ******************************************************************** */
 template <class T> 
inline void import_vector(vector<T>* p_vec, const string& filename,
                            char delimiter = ',') {
    ifstream infile;
	infile.open(filename.c_str());
    if (!infile) {
        throw "Unable to load a vector from the given file:" + filename;
    }
    p_vec->clear();
    while(infile) {
        T new_val;
        infile >> new_val;
        p_vec->push_back(new_val);
        infile.ignore(delimiter);
    }
	infile.close();
}

/* *********************************************************************
    Imports the content of the given array from file, seperated with
    the delimeter
    Note 1: previous content of the vecor will be cleared.
    Note 2: Teh aray should already be of approprite size 
 ******************************************************************** */
 template <class T> 
inline void import_array(valarray<T>* p_arr, const string& filename,
                            char delimiter = ',') {
    ifstream infile;
	infile.open(filename.c_str());
    if (!infile) {
        throw "Unable to load a vector from the given file:" + filename;
    }
    unsigned int i = 0;
    while(true) {
        T new_val;
        char delim;
        if (!(infile >> new_val)) {
            break;
        }
        infile >> delim;
        (*p_arr)[i] = new_val;
        i++;
    }
    cout << "i = " << i <<  " --- size = " << p_arr->size() << endl;
    assert (i == p_arr->size());
	infile.close();
}

/* *********************************************************************
    Imports the content of the given Matrix from file. The matrix format
    is as follows:
        width,height\n
        M[0,0], M[0,1], ..., M[0,n]\n
        M[1,0], M[1,1], ..., M[1,n]\n
        ...
        M[m,0], M[m,1], ..., M[m,n]\n
    Note: previous content of the Matrix will be cleared.
 ******************************************************************** */
 template <class T> 
inline void import_matrix(vector< vector<T> >* pm_matrix, const string& filename){
    string err_str = "Unable to load a vector from the given file:" + filename;
    ifstream infile;
	infile.open(filename.c_str());
    if (!infile) {
        throw err_str.c_str();
    }
    pm_matrix->clear();
    char delim;
    int width, height;
    infile >> width;
    infile.get(delim);
    if (delim != ',') {
        throw err_str.c_str();
    }
    infile >> height;
    infile.get(delim);
    if (delim != '\n') {
        throw err_str.c_str();
    }
    if (width <= 0 || height <= 0) {
        throw err_str.c_str();
    }
    // Import the shape matrix
    int in_val;
    for(int i = 0; i < height; i++) {
        IntVect row;
        for(int j = 0; j < width; j++) {
            infile >> in_val;
            infile.get(delim);
            if ((j < (width - 1) && delim != ',') ||
                (j == (width - 1) && delim != '\n'))  {
                throw err_str.c_str();
            }
            row.push_back(in_val);
        }
        pm_matrix->push_back(row);
    }
}

/* *********************************************************************
    Exports the content of the given Matrix to file. The matrix format
    is as follows:
        width,height\n
        M[0,0], M[0,1], ..., M[0,n]\n
        M[1,0], M[1,1], ..., M[1,n]\n
        ...
        M[m,0], M[m,1], ..., M[m,n]\n
 ******************************************************************** */
 template <class T> 
inline void export_matrix(	vector< vector<T> >* pm_matrix, 
							const string& filename) {
	assert (pm_matrix->size() > 0);
    ofstream file;
	file.open(filename.c_str());
	file << (*pm_matrix)[0].size() << "," <<  pm_matrix->size() << endl;
	for (unsigned long i = 0; i < pm_matrix->size(); i++) {
		for (unsigned long j = 0; j < (*pm_matrix)[i].size(); j++) {
			 file << (*pm_matrix)[i][j];
			 if (j < (*pm_matrix)[i].size() - 1) {
				file << ",";
			}

		}
		file << endl;
	}
	file.close();
}
#endif

