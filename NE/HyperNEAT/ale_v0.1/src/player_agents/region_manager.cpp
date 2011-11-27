/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  region_manager.h
 *
 *	Implementation of the RegionManager class, which takes care of extracting
 *	regions from screenshots, and merging the regions that belong to the same
 *  objects
 **************************************************************************** */

#include <sstream>
#include "game_controller.h"
#include "vector_matrix_tools.h"
#include "region_manager.h"


RegionManager::RegionManager(OSystem* _osystem, IntMatrix* background_matrix) {
  p_osystem = _osystem;
  pm_background_matrix = background_matrix;
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
	
  // initilize the region matrix
  pm_region_matrix = new IntMatrix;
  for (int i = 0; i < i_screen_height; i++) {
    IntVect row;
    for (int j = 0; j < i_screen_width; j++) {
      row.push_back(-1);
    }
    pm_region_matrix->push_back(row);
  }
  i_curr_num_regions = 0;
  i_prev_num_regions = 0;
	
  // initlize the object vectors
  pv_curr_objects = new RegionObjectList;
  pv_prev_objects = new RegionObjectList;
  for (int i = 0; i < MAX_NUM_OBJECTS; i++) {
    pv_curr_objects->push_back(NULL);
    pv_prev_objects->push_back(NULL);
  }
  pv_curr_merged_objects = new RegionObjectList;
  pv_prev_merged_objects = new RegionObjectList;
		
  b_plot_region_matrix_pre_merge = p_osystem->settings().getBool(
                                                                 "plot_region_matrix_pre_merge");
  b_plot_region_matrix_post_merge = p_osystem->settings().getBool(
                                                                  "plot_region_matrix_post_merge");
  f_max_perc_difference = p_osystem->settings().getFloat(
                                                         "max_perc_difference", true);
  i_max_obj_velocity = p_osystem->settings().getInt("max_obj_velocity", true); 												
  f_max_shape_area_dif = p_osystem->settings().getFloat(
                                                        "max_shape_area_dif", true);
}

RegionManager::~RegionManager() {
  delete pm_region_matrix;
  clear_list_of_pointers(pv_curr_objects);
  clear_list_of_pointers(pv_prev_objects);
  clear_list_of_pointers(pv_curr_merged_objects);
  clear_list_of_pointers(pv_prev_merged_objects);
  delete pv_curr_objects;
  delete pv_prev_objects;
  delete pv_curr_merged_objects;
  delete pv_prev_merged_objects;
}

/* *********************************************************************
   Given a new scree_matrix, it first uses a naive blob-detection 
   method to extract a set of regions. It then looks at the regions in 
   previous screen, calculates the velocity for each region,
   and merges the regions that are adjacent and have the same velocity
   The detected regions are returned as a a list of RegionObjects
   ******************************************************************** */
const RegionObjectList* RegionManager::extract_objects_from_new_screen(
                                                                       const IntMatrix* screen_matrix,
                                                                       int frame_number) {
  swap_curr_and_prev_region_objects();
  // 1- Use our naive method to extract regions
  i_curr_num_regions = extract_regions(screen_matrix);
  if (i_curr_num_regions > MAX_NUM_OBJECTS) {
    stringstream err;
    err << "RegionManager: number of discovered regions (" 
        << i_curr_num_regions << ") is higher than MAX_NUM_OBJECTS ("
        << MAX_NUM_OBJECTS << ")\n" << endl;
    cerr << err;
    cout << err;
    exit(-1);
  }
	
  if (b_plot_region_matrix_pre_merge) {
    plot_region_matrix("pre_merge", frame_number);
  }
	
  // 2- Assign a RegionObject to each of the etxracted region
  for (int reg_num = 1; reg_num <= i_curr_num_regions; reg_num++) {
    RegionObject* new_object = new RegionObject();
    new_object->extract_from_region_matrix(pm_region_matrix, reg_num);
    if (new_object->b_is_valid) {	// non-empty region
      (*pv_curr_objects)[reg_num] = new_object;	
      // Find the corresponding object in previous frame,
      RegionObject* previous_object = new_object->get_closest_obj_in_list(
                                                                          pv_prev_objects, i_prev_num_regions, f_max_shape_area_dif, 
                                                                          f_max_perc_difference, i_max_obj_velocity);
      // Calculate object's velocity
      new_object->p_previous_object = previous_object;
      calc_object_velocity(new_object);
    }
  }
	
  // 3- Merge regions in the region_matrix that are connected and
  // correspond to objects with equal velocity
  merge_equivalent_regions();
	
  if (b_plot_region_matrix_post_merge) {
    plot_region_matrix("post_merge", frame_number);
  }
	
  // 4- Generate a new list of merged objects
  for (int reg_num = 1; reg_num <= i_curr_num_regions; reg_num++) {
    RegionObject* new_object = new RegionObject();
    new_object->extract_from_region_matrix(pm_region_matrix, reg_num);
    if (!new_object->b_is_valid) {	
      continue; // empty region
    }
    if (new_object->i_width == 1 && 
        new_object->i_height == 1) {
      continue; // ignore objects consisting of exactly one pixel
    }
    RegionObject* premerge_obj = (*pv_curr_objects)[reg_num];
    assert(premerge_obj != NULL);
    new_object->i_velocity_x = premerge_obj->i_velocity_x;
    new_object->i_velocity_y = premerge_obj->i_velocity_y;
    pv_curr_merged_objects->push_back(new_object);			
  }
  return pv_curr_merged_objects;
}

/* *********************************************************************
   Uses a simple sequential method to categorize non-background pixels
   in the given screen as belonging to one of many discrete regions.
   The result of this method is a matrix, where the item representing
   each pixel is either 0 (i.e. background) or i, where is the region
   number. Returns the number of regions found (pre merge).
	
	
   This sequential approach is taken from Wikipedia
   (http://en.wikipedia.org/wiki/Blob_extraction)
	
   Scan the image from left to right and from top to bottom:
   For every pixel: 
   *   check the north and west pixel (when considering
   4-connectivity) or the northeast, north, northwest, and west 
   pixel for 8-connectivity for a given region criterion.
   *   If none of the neighbors fit the criterion then assign to region
   value of the region counter. Increment region counter.
   *   If only one neighbor fits the criterion assign pixel to that 
   region.
   *   If multiple neighbors match and are all members of the same 
   region, assign pixel to their region.
   *   If multiple neighbors match and are members of different 
   regions, assign pixel to one of the regions 
   Indicate that all of these regions are the equivalent.
	
   Note 1: After this method, the image still needs to be rescanned,
   so the equivalent regions are merged
	
   Note 2: At this point we only find mono-color regions
   ******************************************************************** */
int RegionManager::extract_regions(const IntMatrix* screen_matrix) {
  int num_neighbors = 4;
  int neighbors_y[] = {-1, -1, -1,  0};

  int neighbors_x[] = {-1,  0,  1, -1};
  // Reset the region-matrix
  for (int i = 0; i < i_screen_height; i++) {
    for (int j = 0; j < i_screen_width; j++) {
      (*pm_region_matrix)[i][j] = -1;
    }
  }
  int region_counter = 1; // region 0 is reseved for the background
	
  // 1- First Scan
  int i, j, y, x, color_ind, neighbors_ind, found_region;
  for (i = 0; i < i_screen_height; i++) {
    for (j = 0; j < i_screen_width; j++) {
      color_ind = (*screen_matrix)[i][j];
      if (color_ind == (*pm_background_matrix)[i][j]) {
        // This pixel is part of the background
        (*pm_region_matrix)[i][j] = 0;
        continue;
      }
      // find the region of i,j based on west and north neighbors.
      found_region = -1; 
      for (neighbors_ind = 0; neighbors_ind < num_neighbors;
           neighbors_ind++) {
        y = i + neighbors_y[neighbors_ind];
        x = j + neighbors_x[neighbors_ind];
        if (x < 0 || x >= i_screen_width ||
            y < 0 || y >= i_screen_height) {
          continue;
        }
        if ((*pm_region_matrix)[y][x] != 0 &&
            (*pm_region_matrix)[y][x] != -1 &&
            (*screen_matrix)[y][x] == color_ind) {
          found_region = (*pm_region_matrix)[y][x];
          break;
        }
      }
      if (found_region == -1) {
        // this pixel is in a new region
        (*pm_region_matrix)[i][j] = region_counter;
        region_counter++;
      } else {
        (*pm_region_matrix)[i][j] = found_region;
      }
    }
  }
  // 2- Re-scan the region_matrix, and merge equivalent regions
  int my_region, nb_region, my_color_ind, nb_color_ind;
  int nb_x, nb_y, ind_i, ind_j, lower_region, higher_region;
  for (i = 0; i < i_screen_height; i++) {
    for (j = 0; j < i_screen_width; j++) {
      my_region = (*pm_region_matrix)[i][j];
      if (my_region == 0) {
        continue; // background pixel
      }
      my_color_ind = (*screen_matrix)[i][j];
      for (y = -1; y <= 1; y++) {
        for (x = -1; x <= 1; x++) {
          nb_y = i + y;
          nb_x = j + x;
          if (nb_x < 0 || nb_x >= i_screen_width ||
              nb_y < 0 || nb_y >= i_screen_height) {
            continue;
          }
          nb_region = (*pm_region_matrix)[nb_y][nb_x];
          if (nb_region == 0) {
            continue;
          }
          nb_color_ind = (*screen_matrix)[nb_y][nb_x];
          if (nb_color_ind == my_color_ind &&
              nb_region != my_region) {
            // These two regions are equivilant
            if (my_region > nb_region) {
              higher_region = my_region;
              lower_region = nb_region;
            } else {
              higher_region = nb_region;
              lower_region = my_region;
            }
            // Go through the region matrix and convert
            // higher_region to lower_region
            for (ind_i = 0; ind_i < i_screen_height; ind_i++) {
              for (ind_j = 0; ind_j < i_screen_width; ind_j++) {
                if ((*pm_region_matrix)[ind_i][ind_j] == higher_region) {
                  (*pm_region_matrix)[ind_i][ind_j] = lower_region;
                }
              }
            }
          }    
        }
      }
    }
  }
  return region_counter - 1;
}

/* *********************************************************************
   Merges regions in the region_matrix that are connected and
   correspond to objects with equal velocity
   ******************************************************************** */
void RegionManager::merge_equivalent_regions(void) {
  int i, j, shift_x, shift_y, y, x, ind_i, ind_j;
  int my_region_num, neighbor_region_num;
  RegionObject *my_obj, *nb_obj;
  int high_region_num, low_region_num;
  for (i = 0; i < i_screen_height; i++) {
    for (j = 0; j <i_screen_width; j++) {
      my_region_num = (*pm_region_matrix)[i][j];
      if (my_region_num == 0) {
        continue;   // ignore the background pixels
      }
      my_obj = (*pv_curr_objects)[my_region_num];
      if (my_obj == NULL) {
        continue; // invalid object
      }
      for(shift_y = -3; shift_y <= 3; shift_y++) {
        for (shift_x = -3; shift_x <= -3; shift_x++) {
          y = i + shift_y;
          x = j + shift_x;
          if (x < 0 || x >= i_screen_width ||
              y < 0 || y >= i_screen_height) {
            continue;
          }
          neighbor_region_num = (*pm_region_matrix)[y][x];
          if (neighbor_region_num == 0) {
            continue;   // ignore the background pixels
          }
          nb_obj = (*pv_curr_objects)[neighbor_region_num];
          if (nb_obj == NULL) {
            continue; // invalid object
          }
          if ((my_region_num != neighbor_region_num)  &&
              (my_obj->i_velocity_x == nb_obj->i_velocity_x) &&
              (my_obj->i_velocity_y == nb_obj->i_velocity_y)) {
            // These two regions should be merged
            if (my_region_num > neighbor_region_num) {
              high_region_num = my_region_num;
              low_region_num = neighbor_region_num;
            } else {
              high_region_num = neighbor_region_num;
              low_region_num = my_region_num;
            }
            // Go through the region matrix and convert
            // high_region_num to low_region_num
            for (ind_i = 0; ind_i < i_screen_height; ind_i++) {
              for (ind_j = 0; ind_j < i_screen_width; ind_j++) {
                if ((*pm_region_matrix)[ind_i][ind_j] == 
                    high_region_num) {
                  (*pm_region_matrix)[ind_i][ind_j] = 
                    low_region_num;
                }
              }
            }
          }
        }
      }
    }
  }
}


/* *********************************************************************
   Calculates the object velocity, based on its location, and the
   location of its coressponding  object in previous frame. If there
   is no previous object assigned, will set velocity to 0.
   The velocity is in pixels/frame unit
   ******************************************************************** */
void RegionManager::calc_object_velocity(RegionObject* obj) {
  if (obj->p_previous_object == NULL) {
    obj->i_velocity_x = 0;
    obj->i_velocity_y = 0;
  } else {
    int x_dist = obj->i_center_x - obj->p_previous_object->i_center_x;
    int y_dist = obj->i_center_y - obj->p_previous_object->i_center_y;
    obj->i_velocity_x = x_dist;
    obj->i_velocity_y = y_dist;
  }
}

	
/* *********************************************************************
   Swaps  pv_curr_objects with pv_prev_objects
   also swaps pv_c
   ******************************************************************** */
void RegionManager::swap_curr_and_prev_region_objects(void) {
  clear_list_of_pointers(pv_prev_objects);
  RegionObjectList* pv_tmp = pv_prev_objects;
  pv_prev_objects = pv_curr_objects;
  pv_curr_objects = pv_tmp; // this is now a brand new, all NULL, list
  i_prev_num_regions = i_curr_num_regions;
  i_curr_num_regions = 0;
  clear_list_of_pointers(pv_prev_merged_objects);
  delete pv_prev_merged_objects;
  pv_prev_merged_objects = pv_curr_merged_objects;
  pv_curr_merged_objects = new RegionObjectList;

}   



/* *********************************************************************
   Plots the region manager.
   pre_post is either "pre_merge" or "psot_merge" and used in filename
   ******************************************************************** */
void RegionManager::plot_region_matrix(string pre_post, int frame_number) {
  // copy the region_matrix to a new matrix with color indecies
  IntMatrix* region_matrix_copy = new IntMatrix;
  for (int i = 0; i < i_screen_height; i++) {
    IntVect row;
    for (int j = 0; j < i_screen_width; j++) {
      int region_num = (*pm_region_matrix)[i][j];
      if (region_num == 0) {
        row.push_back(BLACK_COLOR_IND + 256);	// background is black
      } else {
        row.push_back(region_num + 256); // use the custom pallete
      }
    }
    region_matrix_copy->push_back(row);
  }
  ostringstream filename;
  char buffer [50];
  sprintf (buffer, "%09d", frame_number);
  filename << "region_matrx__" << pre_post <<  "__frame_" << buffer << ".png";
  p_osystem->p_export_screen->save_png(
                                       region_matrix_copy, filename.str());
  delete region_matrix_copy;
}
