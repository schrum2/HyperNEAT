/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  class_discovery.h
 *
 *  The implementation of the ClassDiscovery class, which is responsible for
 *  discovering classes of objects from screens
 **************************************************************************** */

#ifndef CLASS_DISCOVERY_H
#define CLASS_DISCOVERY_H

#include "common_constants.h"
#include "OSystem.hxx"
#include "region_manager.h"
#include "blob_object.h"
#include "blob_class.h"

typedef vector < BlobClass* > BlobClassList;

class ClassDiscovery  {
  /* *************************************************************************
     Responsible for detecting the background from the given screens
     In the current implementation we use a simple frequency method, i.e.
     we choose the most frequent color for each pixel as the background
        
     Instance variabls:
     - p_region_manager			Pointer to the region-manager object
     - pm_background_matrix		Background matrix
     - i_num_screens				number of screens we have seen
     - i_max_num_screens			Number of frames to use for class-discovery
     - pv_curr_screen_objects	list of objects in the current screen
     - pv_prev_screen_objects	list of objects in the previous screen
     - pv_discovered_classes		list of discovered classes
     - f_max_perc_difference		The maximum percentage of pixels of the two 
     shapes that can be different and the 
     shapes still be considered similar
     - i_max_obj_velocity		Maximum velocity (pixel/sec) of objects on 
     screen 
     - f_max_shape_area_dif		How much two shape areas can differ, and 
     the distance between them still be 
     meaningful. 
     - f_min_on_frame_ratio		Minimum ratio that a class needs to be on 
     screen to be considered a valid class
     - i_min_boundary_length		For a class to be valid, either it has to
     expand at least i_min_boundary_length on
     X or Y axis, or the area it expands must be
     larger than  i_min_boundary_length ^ 2
     - i_max_num_classes			Maximum acceptable number of classes 
     that can be discovered
     - b_plot_pre_filter_classes When true, we will plot the pre-filter 
     discovered classes
     - b_plot_post_filter_classes When true, we will plot the post-filter 
     discovered classes
     ************************************************************************* */

 public:
  ClassDiscovery(OSystem* _osystem);
  virtual ~ClassDiscovery();

  /* *********************************************************************
     Recieves a new screen for class discovery
     ******************************************************************** */
  virtual void get_new_screen(const IntMatrix* screen_matrix, 
                              int frame_number);

  /* *********************************************************************
     Returns true when the classes are extracted and saved
     ******************************************************************** */
  bool is_class_discovery_complete() {
    return (i_num_screens > i_max_num_screens);
  }
		 
 protected:
  /* *********************************************************************
     Check swhetehr the object (or one of its flipped or resized versions)
     belong to the class
     ******************************************************************** */
  bool object_belongs_to_class(const BlobClass* cls, 
                               const RegionObject* obj,
                               bool& flip_horz, bool& flip_vert, 
                               bool& diff_size);


  /* *********************************************************************
     filters out 'noise classes' (i.e. classes that do not appear on the
     screen often enough or do not move in a large enough boundary)
     ******************************************************************** */
  void filter_classes(void);
		
  /* *********************************************************************
     merges similar classes (used when we  have too many classes )
     ******************************************************************** */
  void merge_classes(void);

  /* *********************************************************************
     Plots the shape_matrix's of each class in a subdirectory
     post_filter should be true if this is called after filtering classes
     post_merger should be true if this is called after merging classes
     ******************************************************************** */
  void plot_classes(bool post_filter, bool post_merge);
		
  /* *********************************************************************
     Sorts the region-objects of the discovered classes based
     on their size, and exports the results to a txt file, so that
     it can imported by ClassShape::import_shape_list
     File format is:
     number_of_classes,number_of_shapes\n
     shape_matrix_text_1 (see the comments on constructor) \n
     shape_matrix_text_2\n
     ...
     shape_matrix_text_n
     ******************************************************************** */
  void export_class_shapes(void);

  OSystem* p_osystem;
  IntMatrix* pm_background_matrix;
  RegionManager* p_region_manager;
  const RegionObjectList* pv_curr_screen_objects;
  const RegionObjectList* pv_prev_screen_objects;
  BlobClassList* pv_discovered_classes;
  int i_screen_width;
  int i_screen_height;
  int i_num_screens;
  int i_max_num_screens;
  float f_max_perc_difference;
  int i_max_obj_velocity;
  float f_max_shape_area_dif;		
  float f_min_on_frame_ratio;
  int i_min_boundary_length;
  int i_max_num_classes;
  bool b_plot_pre_filter_classes;
  bool b_plot_post_filter_classes;
};
							
#endif
