/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  class_discovery.cpp
 *
 *  The implementation of the ClassDiscovery class, which is responsible for
 *  discovering classes of objects from screens
 **************************************************************************** */


#include "../common/export_tools.h"
#include "blob_object.h"
#include "../common/vector_matrix_tools.h"
#include "../emucore/FSNode.hxx"
#include "../control/game_controller.h"
#include "class_discovery.h"

ClassDiscovery::ClassDiscovery(OSystem* _osystem) {
  p_osystem = _osystem;
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
  Settings& settings = p_osystem->settings();
  f_max_perc_difference = settings.getFloat("max_perc_difference", true);
  i_max_obj_velocity = settings.getInt("max_obj_velocity", true); 												
  f_max_shape_area_dif = settings.getFloat("max_shape_area_dif", true);
  i_max_num_screens = settings.getInt("cls_disc_frames_num", true);
  f_min_on_frame_ratio = settings.getFloat("min_on_frame_ratio", true);
  i_min_boundary_length = settings.getInt("min_boundary_length", true);
  i_max_num_classes = settings.getInt("max_num_classes", true);
  b_plot_pre_filter_classes = settings.getBool("plot_pre_filter_classes", true);
  b_plot_post_filter_classes = settings.getBool("plot_post_filter_classes", true);
  i_num_screens = 0;
  // Load the background matrix
  pm_background_matrix = new IntMatrix;
  import_matrix(pm_background_matrix, "backgrounds/background_matrix.txt");
  // Initialize the Rigion-Manager
  p_region_manager = new RegionManager(p_osystem, pm_background_matrix);
  // Initilize object/class lists
  pv_curr_screen_objects = NULL;
  pv_prev_screen_objects = NULL;
  pv_discovered_classes = new BlobClassList;
}

ClassDiscovery::~ClassDiscovery() {
  delete p_region_manager;
  delete pm_background_matrix;
  clear_list_of_pointers(pv_discovered_classes);
  delete pv_discovered_classes;
}


/* *********************************************************************
   Recieves a new screen for class discovery
   ******************************************************************** */
void ClassDiscovery::get_new_screen(const IntMatrix* screen_matrix, 
                                    int frame_number) {
  if (frame_number < 1000) {
    return; // ignore the first 1000 frames, since some games act wiered
  }
  if (i_num_screens > i_max_num_screens || pv_discovered_classes->size() > 200) {
    return; // we are done with class discovery
  }
  pv_prev_screen_objects = pv_curr_screen_objects; // RegionManager takes care
  // of their clean-up
  pv_curr_screen_objects = p_region_manager->extract_objects_from_new_screen(
                                                                             screen_matrix, frame_number);

  for (unsigned int i = 0; i <  pv_curr_screen_objects->size(); i++) {
    RegionObject* obj = (*pv_curr_screen_objects)[i];
    // 1- See if this object belongs to an already discovered class
    bool obj_in_class_list = false;
    for (unsigned int c = 0; c < pv_discovered_classes->size(); c++) {
      BlobClass* disc_class = (*pv_discovered_classes)[c];
      bool flip_horz, flip_vert, diff_size;
      if (object_belongs_to_class(disc_class, obj, flip_horz, 
                                  flip_vert, diff_size)) {
        obj_in_class_list = true;
        disc_class->i_num_frames_on_scr++;
        disc_class->update_screen_boudnaries(obj);
        obj->i_instance_of_class = c;
        if (flip_horz || flip_vert || diff_size) {
          // A flipped version of the object or a 
          //version with diferent size also belongs to 
          //class. Add the non-flipped/resized verison
          disc_class->add_object(obj, frame_number, 
                                 "Filpped and/or Different Size");
        } else {
          // The exact object belongs to this class
          // add one to its frame counter
          disc_class->increase_obj_onscreen_count(obj);
        }
        break; // we are done with the inner loop
      }
    }
    if (obj_in_class_list) {
      continue;  // We have already assigned a class to this object
    }
		
    // 2- See if there is a 'similar' object 'close' to this object
    //    in the previous frame
    RegionObject* prev_obj = NULL;
    if (pv_prev_screen_objects) {
      prev_obj = obj->get_closest_obj_in_list(
                                              pv_prev_screen_objects, pv_prev_screen_objects->size(), 
                                              f_max_shape_area_dif, f_max_perc_difference, 
                                              i_max_obj_velocity);
    }
    if (prev_obj) {
      // There was a similar object in the previous frame
      // Assign this object to the same class
      obj->i_instance_of_class = prev_obj->i_instance_of_class;
      BlobClass* target_class = (*pv_discovered_classes)[
                                                         prev_obj->i_instance_of_class];
      string disc_str = "Sim-Obj on Previous Frame";
      target_class->add_object(obj, frame_number, disc_str);
      target_class->i_num_frames_on_scr++;
      continue;
    }
		
    // 3- This is a brand new object, belonging to a brand new class
    BlobClass* new_class = new BlobClass();
    new_class->i_discovered_on_frame = frame_number;
    new_class->i_num_frames_on_scr = 1;
    string disc_str = "Brand New";
    new_class->add_object(obj, frame_number, disc_str);
    obj->i_instance_of_class = pv_discovered_classes->size();
    pv_discovered_classes->push_back(new_class);
    if (pv_discovered_classes->size() > 200) {
      cout << "Warning: number of discovered classes exceeds 200." 
           << "The remaining classes wil be ignored" << endl;
    }
  }
	
  if (i_num_screens == i_max_num_screens || pv_discovered_classes->size() > 200) {
    if (b_plot_pre_filter_classes) {
      cout << "Plotting the (pre-filter) discovered classes...";
      plot_classes(false, false);
      cout << " done." << endl;
    }
    cout << "Filtering classes...";
    filter_classes();
    cout << " done." << endl;
    cout << "Plotting the (post-filter) discovered classes...";
    plot_classes(true, false);
    cout << " done." << endl;
    if (pv_discovered_classes->size() > i_max_num_classes) {
      cout << "Merging together classes...";
      merge_classes();
      cout << " done." << endl;
      if (b_plot_post_filter_classes) {
        cout << "Plotting the (post-merge) discovered classes...";
        plot_classes(true, true);
        cout << " done." << endl;
      }
    }
    cout << "Exporting discovered classes...";  
    export_class_shapes();
    cout << " done." << endl;
  }
  i_num_screens++;
}

/* *********************************************************************
   filters out 'noise classes' (i.e. classes that do not appear on the
   screen often enough or do not move in a large enough boundary)
   ******************************************************************** */
void ClassDiscovery::filter_classes(void) {
  BlobClassList* pv_filtered_classes = new BlobClassList;
  int min_boundary_len_sq = (i_min_boundary_length * i_min_boundary_length);
  int filtered_classes_counter = 0;
  for (int c = 0; c < pv_discovered_classes->size(); c++) {
    BlobClass* curr_class = (*pv_discovered_classes)[c];
    float on_frame_ratio =	(float)curr_class->i_num_frames_on_scr / 
      (float)i_num_screens;
    bool keep_class = true;
    if (on_frame_ratio < f_min_on_frame_ratio) {
      keep_class = false;
    }
    int delta_x =	curr_class->i_screen_pos_x_max - 
      curr_class->i_screen_pos_x_min;
    int delta_y =	curr_class->i_screen_pos_x_max - 
      curr_class->i_screen_pos_x_min;
    if (delta_x < i_min_boundary_length &&
        delta_y < i_min_boundary_length &&
        (delta_x * delta_y) < min_boundary_len_sq) {
      keep_class = false;
    }
    if (keep_class) {
      pv_filtered_classes->push_back(curr_class);
      curr_class->update_class_number(filtered_classes_counter);
      filtered_classes_counter++;
    } else {
      delete curr_class;
      (*pv_discovered_classes)[c] = NULL;
    }
  }
  BlobClassList* to_be_deleted = pv_discovered_classes;
  pv_discovered_classes = pv_filtered_classes;
  delete to_be_deleted;
}

/* *********************************************************************
   merges similar classes (used when we  have too many classes )
   This will continue, until number classes is less than i_max_num_classes
   ******************************************************************** */
void ClassDiscovery::merge_classes(void) {
  int num_merges = 0;
  while (pv_discovered_classes->size() > i_max_num_classes) {
    // 1- Find the two closest classes and merge them
    float closest_dist = -1;
    int merge_ind_a = -1;
    int merge_ind_b = -1;
    for (int i = 0; i < pv_discovered_classes->size(); i++) {	
      for (int j = i + 1; j < pv_discovered_classes->size(); j++) {	
        float dist;
        bool dist_valid = (*pv_discovered_classes)[i]->get_min_distance(	
                                                                        (*pv_discovered_classes)[j], 
                                                                        f_max_shape_area_dif, dist);
        if (!dist_valid) {
          continue;
        }
        if (closest_dist < 0 || dist < closest_dist) {
          closest_dist = dist;
          merge_ind_a = i;
          merge_ind_b = j;
        }
      }
    }
    if (merge_ind_a == -1 || merge_ind_b == -1) {
      cout << "Unable to merge classes down to "<< i_max_num_classes
           << ". The remaining classes are not comparable in distance"
           << endl;
      break;
    }
    // 2- Merge them!
    (*pv_discovered_classes)[merge_ind_b]->update_class_number(merge_ind_a);
    (*pv_discovered_classes)[merge_ind_a]->merge_with(
                                                      (*pv_discovered_classes)[merge_ind_b]);
    delete (*pv_discovered_classes)[merge_ind_b];
    (*pv_discovered_classes)[merge_ind_b] = NULL;
    BlobClassList::iterator delete_it = pv_discovered_classes->begin() + 
      merge_ind_b;
    assert ( (*delete_it) == NULL);
    pv_discovered_classes->erase(delete_it);
    for (int i = merge_ind_b; i < pv_discovered_classes->size(); i++) {
      (*pv_discovered_classes)[i]->update_class_number(i);
      // const RegionObjectList* class_objects = (*pv_discovered_classes)[i]->get_reg_objects();
      // for (int j = 0; j < class_objects->size(); j++) {
      // 	cout << "i:" << i << ", j:" << j << "  |"  << (*class_objects)[j]->export_object_as_txt() << endl;
      // }
    }
    num_merges++;
  }
  cout << "Merged " << num_merges << " classes." << endl;
}

/* *********************************************************************
   Plots the shape_matrix's of each class in a subdirectory
   post_filter should be true if this is called after filtering classes
   post_merger should be true if this is called after merging classes
   ******************************************************************** */
void ClassDiscovery::plot_classes(bool post_filter, bool post_merge) {
  string base_dir = "disc_classes__";
  if (post_merge) {
    base_dir += "post_merge";
  } else if (post_filter) {
    base_dir += "post_filter";
  } else {
    base_dir += "pre_filter";
  }
  if(!FilesystemNode::dirExists(base_dir)) {
    FilesystemNode::makeDir(base_dir);
  }
	
  for (int c = 0; c < pv_discovered_classes->size(); c++) {
    BlobClass* curr_class = (*pv_discovered_classes)[c];
    char buffer[50];
    sprintf (buffer, "%03d", c);
    string sub_dir = base_dir + "/" + "class_" + buffer;
    if(!FilesystemNode::dirExists(sub_dir)) {
      FilesystemNode::makeDir(sub_dir);
    }
    // Write the class info
    string info_file = sub_dir + "/class_info.txt";
    ofstream file;
    file.open(info_file.c_str());
    file << curr_class->get_info_text(i_num_screens) << endl; 
    file.close();
    // Plot and write info for each RegionObject in this class
    for (int j = 0; j < curr_class->pv_reg_objects->size(); j++) {
      RegionObject* curr_obj = (*curr_class->pv_reg_objects)[j];
      sprintf (buffer, "%04d", j);
      info_file = sub_dir + "/object_info_" + buffer + ".txt";
      file.open(info_file.c_str());
      file << curr_obj->get_info_text() << endl; 
      file.close();
      string plot_file = sub_dir + "/object_info_" + buffer + ".png";
      curr_obj->plot(plot_file, 
                     p_osystem->p_export_screen);
    }
  }
}

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
void ClassDiscovery::export_class_shapes(void) {
  RegionObjectList sorted_objects;
  for (int c = 0; c < pv_discovered_classes->size(); c++) {
    BlobClass* curr_class = (*pv_discovered_classes)[c];
    for (int o = 0; o < curr_class->pv_reg_objects->size(); o++) {
      sorted_objects.push_back((*curr_class->pv_reg_objects)[o]);
    }
  }
  sort(sorted_objects.begin(), sorted_objects.end(), 
       compare_objects_by_shape_size);
  int num_shapes = sorted_objects.size();
  int num_classes = pv_discovered_classes->size();
  ofstream file;
  file.open("class_shapes.txt");
  file << num_classes << "," << num_shapes << endl;
  for (int i = 0; i < num_shapes; i++) {
    file << sorted_objects[i]->export_object_as_txt() << endl; 
  }
}

/* *********************************************************************
   Check swhetehr the object (or one of its flipped or resized versions)
   belong to the class
   ******************************************************************** */
bool ClassDiscovery::object_belongs_to_class(const BlobClass* cls, 
                                             const RegionObject* obj,
                                             bool& flip_horz, bool& flip_vert, 
                                             bool& diff_size) {
  for (int do_flip_horz = 0; do_flip_horz <= 1; do_flip_horz++) {
    for (int do_flip_vert = 0; do_flip_vert <= 1; do_flip_vert++) { 
      for (int do_diff_size = 0; do_diff_size <= 1; do_diff_size++) { 
        flip_horz = (bool)do_flip_horz;
        flip_vert = (bool)do_flip_vert;
        diff_size = (bool)do_diff_size;
        if (cls->belongs(obj, diff_size, flip_horz, 
                         flip_vert, f_max_shape_area_dif)) {
          return true;
        }
      }
    }
  } 
  return false;
}
