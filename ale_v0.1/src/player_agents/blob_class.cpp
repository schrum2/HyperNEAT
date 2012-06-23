/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  blob_class.h
 *
 *  Implementation of the BlobClass class, which represents a class of  objects
 *	detected in game screens. This should not be confused with ClassShape,
 *  which represents a single shape belonging to a BlobClass
 **************************************************************************** */

#include <sstream>
#include "blob_class.h"
#include "shape_tools.h"
#include "../common/vector_matrix_tools.h"
#include "common_constants.h"


BlobClass::BlobClass() {
	pv_reg_objects = new RegionObjectList;
	i_num_frames_on_scr = 0;
	i_screen_pos_x_min = -1;
	i_screen_pos_x_max = -1;
	i_screen_pos_y_min = -1;
	i_screen_pos_y_max = -1;
	i_discovered_on_frame = -1;
}

BlobClass::~BlobClass() {
	clear_list_of_pointers(pv_reg_objects);
	delete pv_reg_objects;
}

/* *********************************************************************
 * Adds (a copy) of the shape to the add it to the the representative 
 * objects of this class
 ******************************************************************** */
void BlobClass::add_object(RegionObject*  reg_object, int frame_num, 
							string discovery_txt) {
	RegionObject* new_object = new RegionObject(*reg_object);
	new_object->i_disc_frame_num = frame_num;
	new_object->i_num_frames_on = 1;
	new_object->str_disc_text = discovery_txt;
	pv_reg_objects->push_back(new_object);
	update_screen_boudnaries(reg_object);
}
 
/* *********************************************************************
 * Given a new object that belongs to this class, updates our on-screen
 * boundaries.
 ******************************************************************** */
void BlobClass::update_screen_boudnaries(const RegionObject* reg_object) {
	if  (	i_screen_pos_x_min == -1  ||
			reg_object->i_center_x  < i_screen_pos_x_min) {
		i_screen_pos_x_min = reg_object->i_center_x;
	}
	if  (	i_screen_pos_x_max == -1  ||
			reg_object->i_center_x  > i_screen_pos_x_max) {
		i_screen_pos_x_max = reg_object->i_center_x;
	}
	if  (	i_screen_pos_y_min == -1  ||
			reg_object->i_center_y  < i_screen_pos_y_min) {
		i_screen_pos_y_min = reg_object->i_center_y;
	}
	if  (	i_screen_pos_y_max == -1  ||
			reg_object->i_center_y  > i_screen_pos_y_max) {
		i_screen_pos_y_max = reg_object->i_center_y;
	}
}

/* *********************************************************************
 * Returns true if the given object belongs to this class.
 * When check_flipped_horizentally is true, we check if the flipped 
 * version of the object belongs to this class. Same deal with
 *  check_flipped_vertically, and allow_different_size
 ******************************************************************** */
bool BlobClass::belongs(const RegionObject* reg_object, 
						bool allow_different_size,
						bool check_flipped_horizentally, 
						bool check_flipped_vertically, 
						float max_shape_area_dif) const {
	RegionObject*  flipped_obj = NULL;
	if (check_flipped_vertically || check_flipped_horizentally) {
		flipped_obj = new RegionObject(*reg_object);
		flip_shape(reg_object->pm_shape_matrix, flipped_obj->pm_shape_matrix, 
					check_flipped_horizentally, check_flipped_vertically);
	}
	bool do_belong = false;
	for (unsigned int i = 0; i < pv_reg_objects->size(); i++) {
		RegionObject* curr_object = (*pv_reg_objects)[i];
		int pixel_distance = -1;
		float perc_distance = -1.0;
		bool dist_valid;
		if (flipped_obj) {
			dist_valid = curr_object->calc_distance(flipped_obj, 
										allow_different_size, max_shape_area_dif,
										pixel_distance, perc_distance); 
		} else {
			dist_valid = curr_object->calc_distance(reg_object, 
										allow_different_size, max_shape_area_dif,
										pixel_distance, perc_distance); 
		}
        if (dist_valid && pixel_distance == 0) {
			do_belong = true;
			break;
		}
	}
	if (flipped_obj) {
		delete flipped_obj;
	}
	return do_belong; 
}

/* *********************************************************************
 * Adds one to the i_num_frames_on of the corresponding RegionObject 
 *  in our pv_rep_objects 
 ******************************************************************** */
void BlobClass::increase_obj_onscreen_count(const RegionObject* reg_object) {
	for (unsigned int i = 0; i < pv_reg_objects->size(); i++) {
		RegionObject* curr_object = (*pv_reg_objects)[i];
		int pixel_distance = -1;
		float perc_distance = -1.0;
		bool dist_valid = curr_object->calc_distance(reg_object, 
									false, 0.0,
									pixel_distance, perc_distance); 
		if (dist_valid && pixel_distance == 0) {
			assert (curr_object->i_num_frames_on > 0);
			curr_object->i_num_frames_on++;
			return;
		}
	}
	string err = string("BlobClass::increase_obj_onscreen_count() called") + 
				 string(" on object that is not a member of this blob-class\n");
	cerr << err;
	cout << err;
	exit(-1);
}

/* *********************************************************************
 * Merges this class with the given class
 ******************************************************************** */
void BlobClass::merge_with(const BlobClass* other_class) {
	const RegionObjectList* other_objects = other_class->get_reg_objects();
	for (unsigned int i = 0; i < other_objects->size(); i++) {
		RegionObject* new_object = new RegionObject(*((*other_objects)[i]));
		pv_reg_objects->push_back(new_object);
	}
	i_screen_pos_x_max = max(i_screen_pos_x_max, 
							 other_class->i_screen_pos_x_max);
	i_screen_pos_x_min = min(i_screen_pos_x_min, 
							 other_class->i_screen_pos_x_min);
	i_screen_pos_y_max = max(i_screen_pos_y_max, 
							 other_class->i_screen_pos_y_max);
	i_screen_pos_y_min = min(i_screen_pos_x_min, 
							 other_class->i_screen_pos_y_min);
}

/* *********************************************************************
 * Goes through all the region-objects assigned to this class, and 
 * updates their i_instance_of_class
 ******************************************************************** */
void BlobClass::update_class_number(int new_num) {
	for (int i = 0; i < pv_reg_objects->size(); i++) {
		(*pv_reg_objects)[i]->i_instance_of_class = new_num;
	}
}

/* *********************************************************************
 * Returns the minimum (percentage) distance between the objects of two
 * classes
 * Returns false if the two classes donot have a well-defined distance
 ******************************************************************** */
bool BlobClass::get_min_distance(	const BlobClass* other_class, 
									float max_shape_area_dif, 
									float& min_dist) const {
	min_dist = -1.0;
	for (int i = 0; i < pv_reg_objects->size(); i++) {
		RegionObject* my_obj = (*pv_reg_objects)[i];
		for (int j = 0; j < other_class->pv_reg_objects->size(); j++) {		
			RegionObject* other_obj = (*other_class->pv_reg_objects)[j];
			int pixel_distance;
			float perc_distance;
			bool distance_valid = my_obj->calc_distance(other_obj, true, 
														max_shape_area_dif, 
												pixel_distance, perc_distance);
			if (!distance_valid) {
				continue;
			}
			if (min_dist < 0 || perc_distance < min_dist) {
				min_dist = perc_distance;
			}
		}
	}
	if (min_dist < 0) {
		return false;
	}
	return min_dist;
}

/* *********************************************************************
 * Accessor method
 ******************************************************************** */
 const RegionObjectList* BlobClass::get_reg_objects(void) const {
	return pv_reg_objects;
}


/* *********************************************************************
 * Returns a string containing general info about this class. 
 * This is used to save the class info, when plotting the blob-classes
 ******************************************************************** */
 string BlobClass::get_info_text(int num_frames) const {
	float on_frame_ratio = (float)i_num_frames_on_scr / (float)num_frames;
	int delta_x = i_screen_pos_x_max - i_screen_pos_x_min;
	int delta_y = i_screen_pos_y_max - i_screen_pos_y_min;
	int boundary_area = delta_x * delta_y;
	ostringstream info_str;
    info_str << "Number of Farmes on Screen: " << i_num_frames_on_scr << endl 
			 << "On Screen Ratio: " << on_frame_ratio << endl
			 << "Screen Positions Bound: (" << i_screen_pos_x_min << "," 
			 << i_screen_pos_y_min << ") - (" << i_screen_pos_x_max << ","
			 << i_screen_pos_y_max << ")" << endl
			 << "Boundary Area: " << boundary_area << endl 
			 << "Discovered on frame #" << i_discovered_on_frame << endl;
	return info_str.str();
}
