/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  blob_object.cpp
 *
 *  Represents an object detected in a game screen.
 **************************************************************************** */

#include <sstream>
#include "blob_object.h"
#include "vector_matrix_tools.h"
#include "shape_tools.h"

BlobObject::BlobObject( int _center_x, int _center_y, int _velocity_x, 
                        int _velocity_y, int _instance_of_class) {
  i_center_x = _center_x;
  i_center_y = _center_y;
  i_velocity_x = _velocity_x;
  i_velocity_y = _velocity_y;
  i_instance_of_class = _instance_of_class;
}                        

/* *********************************************************************
   Returns the  distance between this object and the given object
   Returns the max(abs(distance_x), abs(distance_y))
   ******************************************************************** */
int BlobObject::calc_distance_max_xy( BlobObject* other_obj) {
  return max( abs(i_center_x - other_obj->i_center_x), 
              abs(i_center_y - other_obj->i_center_y) );
}



/* *****************************************************************************
   class RegionObject 
   Represents an object detected in the region matrix.
   This is used in Class-Discovery 
   ***************************************************************************** */

/* *********************************************************************
   Constructor
   This will generate an in_valid object. User should call
   extract_from_region_matrix to properly initialize this method
   ******************************************************************** */
RegionObject::RegionObject() :
  BlobObject(0, 0) {
  b_is_valid = false;
  pm_shape_matrix = NULL;
  i_region_number = -1;
  p_previous_object = NULL;
  i_disc_frame_num = -1;
  i_num_frames_on = -1;
  str_disc_text = "";
  i_width = i_height = -1;
	
}

/* *********************************************************************
   copy constructor
   ******************************************************************** */
RegionObject::RegionObject(const RegionObject& other_obj) : 
  BlobObject(other_obj.i_center_x, other_obj.i_center_y) {
  b_is_valid = other_obj.b_is_valid;
  i_width = other_obj.i_width;
  i_height = other_obj.i_height;
  pm_shape_matrix = new IntMatrix;
  for (int i = 0; i < i_height; i++) {
    IntVect row;
    for (int j = 0; j < i_width; j++) {
      row.push_back((*other_obj.pm_shape_matrix)[i][j]);
    }
    pm_shape_matrix->push_back(row);
  }
  i_region_number = other_obj.i_region_number;
  p_previous_object = other_obj.p_previous_object;
  i_disc_frame_num = other_obj.i_disc_frame_num;
  i_num_frames_on = other_obj.i_num_frames_on;
  str_disc_text = other_obj.str_disc_text;
}

/* *********************************************************************
   Assignment Operator
   ******************************************************************** */
RegionObject& RegionObject::operator=(const RegionObject& that) {
  assert(false);
  cout << "Assignment Operator is not yet implemented for RegionObject"<<endl;
  cerr << "Assignment Operator is not yet implemented for RegionObject"<<endl;
  exit(-1);
}
		
RegionObject::~RegionObject() {
  if (pm_shape_matrix) {
    delete pm_shape_matrix;
  }
}

/* *********************************************************************
   Given a region matrix (generated in region_manager),
   and a region number, extracts the object from the matrix
   Returns true if the extraction is sucessful
   ******************************************************************** */
bool RegionObject::extract_from_region_matrix(	const IntMatrix* region_matrix, 
                                                int region_num) {
  // if we had assigned an old shape, delete it
  if (pm_shape_matrix) {
    delete pm_shape_matrix;
  }
	
  // Find the indexies in the region, with the given number
  IntVect* pv_row_inds = new IntVect;
  IntVect* pv_col_inds = new IntVect;;
  simple_where(	region_matrix, region_num, pv_row_inds, pv_col_inds);
  int num_pixels = pv_row_inds->size();
  if (num_pixels == 0) {
    // This region number doesn't have any actual pixels on screen
    b_is_valid = false;
    delete pv_row_inds;
    delete pv_col_inds;
    return false;
  }
	
  // find the shape of the region
  int x_min, x_min_ind, x_max, x_max_ind; 
  int y_min, y_min_ind, y_max, y_max_ind; 
  vect_min(pv_col_inds, x_min, x_min_ind);
  vect_max(pv_col_inds, x_max, x_max_ind);
  vect_min(pv_row_inds, y_min, y_min_ind);
  vect_max(pv_row_inds, y_max, y_max_ind);
  i_width = x_max - x_min + 1;
  i_height = y_max - y_min + 1;
  assert (i_width > 0 && i_height > 0); 
	
  // Full the shape matrix with region pixels
  pm_shape_matrix = new IntMatrix;
  for (int i = 0; i < i_height; i++) {
    IntVect row;
    for (int j = 0; j < i_width; j++) {
      row.push_back(0);
    }
    pm_shape_matrix->push_back(row);
  }
  for (int c = 0; c < num_pixels; c++) {
    int i = (*pv_row_inds)[c] - y_min;
    int j = (*pv_col_inds)[c] - x_min;
    assert (i >= 0 && j >= 0 && i < i_height && j < i_width);
    (*pm_shape_matrix)[i][j] = 1;
  }
	
  // Set the rest of variables
  i_region_number = region_num;
  i_center_x = x_min + (int)(i_width / 2);
  i_center_y = y_min + (int)(i_height / 2);
  b_is_valid = true;
  delete pv_row_inds;
  delete pv_col_inds;
  return true;
}
				
/* *********************************************************************
   Calculates the pixel and percentage distance between the two 
   objects. Returns true if the distance is computable (i.e. the 
   objects have compatible shapes).
   ******************************************************************** */
bool RegionObject::calc_distance(	const RegionObject* other_object, 
                                        bool allow_different_size, float max_shape_area_dif,
                                        int& pixel_distance, float& perc_distance) {
  bool success = calc_shape_distance(	pm_shape_matrix, 
                                        other_object->pm_shape_matrix, 
                                        allow_different_size, max_shape_area_dif,
                                        pixel_distance, perc_distance);
  return success;
}


/* *********************************************************************
   Returns the Region-Object closest to this object in the given list of 
   objects.
   We do this by collecting a set of 'similar' objects that apear 
   'close' to the given object in the previous frame, and then return
   the one that is most similar.
   ******************************************************************** */
RegionObject* RegionObject::get_closest_obj_in_list(
                                                    const RegionObjectList* pv_obj_list,
                                                    int list_length,
                                                    float max_shape_area_dif,
                                                    float max_perc_difference,
                                                    int max_obj_velocity) {
  RegionObject* closest_prev_obj = NULL;	// closest object found so far
  int closest_pixel_distance	= -1;		// Their pixel differecne 
  int closest_sq_distance	= -1; // The square of their actual distance
	
  for (int i = 0; i < list_length; i++) {
    RegionObject* prev_obj = (*pv_obj_list)[i];
    if (prev_obj == NULL || prev_obj->b_is_valid == false) {
      continue; // invalid object
    }
    int horizental_dist = abs(	i_center_x - 
                                prev_obj->i_center_x);
    int vertical_dist = abs(	i_center_y - 
                                prev_obj->i_center_y);
		
    if (horizental_dist < max_obj_velocity && 
        vertical_dist < max_obj_velocity) {
      int pixel_distance = -1;
      float perc_distance = -1.0;
			
      bool distance_valid = calc_distance(prev_obj, true, 
                                          max_shape_area_dif, 
                                          pixel_distance, perc_distance);
      if (!distance_valid) { 
        continue;
      }
      if (perc_distance < max_perc_difference) {
        int sq_distance = (horizental_dist*horizental_dist);
        sq_distance += (vertical_dist * vertical_dist);
        if  (	closest_prev_obj == NULL ||
                pixel_distance < closest_pixel_distance ) {
          closest_prev_obj = prev_obj;
          closest_pixel_distance = pixel_distance;
          closest_sq_distance = sq_distance;
        } else if (	pixel_distance == closest_pixel_distance &&
                        closest_sq_distance > sq_distance) {
          // We have found an object that is as similar as
          // closest_prev_obj, but it is physicaly closer to
          // current object
          closest_prev_obj = prev_obj;
          closest_pixel_distance = pixel_distance;
          closest_sq_distance = sq_distance;
        }
      }
    }
  }
  return closest_prev_obj;
}

/* *********************************************************************
   Save the shape_matrix as a PNG file
   ******************************************************************** */
void RegionObject::plot(string& filename, 
                        const ExportScreen* p_export_screen) const {
  IntMatrix* plot_matrix = new IntMatrix;
  for (int i = 0; i < i_height; i++) {
    IntVect row;
    for (int j = 0; j < i_width; j++) {		
      if((*pm_shape_matrix)[i][j]) {
        row.push_back(256 + WHITE_COLOR_IND);
      } else {
        row.push_back(256 + BLACK_COLOR_IND);
      }
    }
    plot_matrix->push_back(row);
  }
  p_export_screen->export_any_matrix (plot_matrix, filename);
  delete plot_matrix;
}

/* *********************************************************************
   Returns the string version of this object, so it can be imported 
   later. The string format is as follows:
   class_num,width,height|
   M[0,0], M[0,1], ..., M[0,n]|
   M[1,0], M[1,1], ..., M[1,n]|
   ...
   M[m,0], M[m,1], ..., M[m,n]|
   ******************************************************************** */
string RegionObject::export_object_as_txt(void) const {
  ostringstream exp_str;
  exp_str << i_instance_of_class << ",";
  exp_str << i_width << "," << i_height <<  "|";
  for (int i = 0; i < i_height; i++) {
    for (int j = 0; j < i_width; j++) {
      exp_str << (*pm_shape_matrix)[i][j];
      if (j < i_width - 1) {
        exp_str << ",";
      } else { 
        exp_str << "|";
      }
    }
  }
  return exp_str.str();
}
	
/* *********************************************************************
 * Returns a string containing general info about this object. 
 * This is used to save object info, after class-discovery
 ******************************************************************** */
string RegionObject::get_info_text(void) const {
  ostringstream info_str;
  info_str << "Discovered on frame: " << i_disc_frame_num << endl
           << "Number of frames on screen: " << i_num_frames_on << endl
           << "Size: " << i_height << "x" << i_width << endl 
           << "Comment: " + str_disc_text << endl; 
  return info_str.str();
}


/* *************************************************************************
   Compares two objects based on their area size. Returns returns true if 
   obj1 > obj2, false otherwise
   ************************************************************************* */
bool compare_objects_by_shape_size(	const RegionObject* obj1, 
                                        const RegionObject* obj2) {
  int obj1_area = obj1->i_width * obj1->i_height;
  int obj2_area = obj2->i_width * obj2->i_height;
  if (obj1_area > obj2_area) {
    return true;
  } else {
    return false;
  }
}
