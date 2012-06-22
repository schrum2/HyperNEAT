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

#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "common_constants.h"
#include "../emucore/OSystem.hxx"
#include "blob_object.h"

#define MAX_NUM_OBJECTS 1000


class RegionManager  {
  /* *************************************************************************
     This class takes care of extracting region-objects from screens,
     and merging the regions that belong to the same object
            
     Object Variables:
     - pm_region_matrix      region_matrix[i,j] determines the
     region number that pixel i,j belongs to.
     Region 0 is reserved for background pixels
     - i_curr_num_regions    Number of regions in the current timestep
     - i_prev_num_regions    Number of regions in the previous timestep
     - v_curr_objects        List of objects detected on current screen
     v_curr_objects[i] holds the object 
     corresponding to rehion i
     - v_prev_objects        List of objects detected on previous screen
     - pv_curr_merged_objects  List of the current merged objects. This 
     one is not indexed by region number
     - pv_prev_merged_objects  List of the previous merged objects. This 
     is kept, so I don;t delete prev_objects
     in ClassDiscovery
     - pm_background_matrix  The background matrix
     - b_plot_region_matrix_pre_merge	When true, we will plot the 
     region matrix, befor mering the objects
     - b_plot_region_matrix_post_merge	When true, we will plot the 
     region matrix, after mering the objects
     - f_max_perc_difference	The maximum percentage of pixels of the two 
     shapes that can be different and the 
     shapes still be considered similar
     - i_max_obj_velocity	Maximum velocity (pixel/sec) of objects on 
     screen 
     - f_max_shape_area_dif	How much two shape areas can differ, and 
     the distance between them still be 
     meaningful. 
     ************************************************************************* */

 public:
  RegionManager(OSystem* _osystem, IntMatrix* background_matrix);
  virtual ~RegionManager();

  /* *********************************************************************
     Given a new scree_matrix, it first uses a naive blob-detection 
     method to extract a set of regions. It then looks at the regions in 
     previous screen, calculates the velocity for each region,
     and merges the regions that are adjacent and have the same velocity
     The detected regions are returned asa a list of RegionObjects
     ******************************************************************** */
  const RegionObjectList* extract_objects_from_new_screen(
                                                          const IntMatrix* screen_matrix, 
                                                          int frame_number);
		
 protected:
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
  int extract_regions(const IntMatrix* screen_matrix);

  /* *********************************************************************
     Merges regions in the region_matrix that are connected and
     correspond to objects with equal velocity
     ******************************************************************** */
  void merge_equivalent_regions(void);
				
  /* *********************************************************************
     Calculates the object velocity, based on its location, and the
     location of its coressponding  object in previous frame. If there
     is no previous object assigned, will set velocity to 0.
     The velocity is in pixels/frame unit
     ******************************************************************** */
  void calc_object_velocity(RegionObject* obj);


  /* *********************************************************************
     Deletes pv_prev_objects and replaces it with pv_curr_objects.
     ******************************************************************** */
  void swap_curr_and_prev_region_objects(void);
		

  /* *********************************************************************
     Plots the region manager.
     pre_post is either "pre_merge" or "psot_merge" and used in filename
     ******************************************************************** */
  void plot_region_matrix(string pre_post, int frame_number);

  OSystem* p_osystem;
  IntMatrix* pm_background_matrix;
  IntMatrix* pm_region_matrix; 
  int i_curr_num_regions;
  int i_prev_num_regions;
  RegionObjectList* pv_curr_objects;
  RegionObjectList* pv_prev_objects;
  RegionObjectList* pv_curr_merged_objects;
  RegionObjectList* pv_prev_merged_objects;
  bool b_plot_region_matrix_pre_merge;
  bool  b_plot_region_matrix_post_merge;
  int i_screen_width;
  int i_screen_height;
  float f_max_perc_difference;
  int i_max_obj_velocity;
  float f_max_shape_area_dif;
};
#endif 					
