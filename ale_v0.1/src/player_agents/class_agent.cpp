/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  class_agent.cpp
 *
 *  The implementation of the ClassAgent class, which is a generic player 
 *  agent that learns to play, based on detecting instances of blob classes
 *  on the screen, and using tile coding on their relative position and velocity
 **************************************************************************** */

#include <sstream>
#include "class_agent.h"
#include "../common/export_tools.h"
#include "../control/game_controller.h"
#include "tiles2.h"
#include "../common/random_tools.h"
#include "../common/misc_tools.h"


ClassAgent::ClassAgent(GameSettings* _game_settings, OSystem* _osystem) :
  PlayerAgent(_game_settings, _osystem) {
  Settings& settings = p_osystem->settings();
  b_inc_abs_positions = settings.getBool("include_abs_positions", true);
  i_num_tilings = settings.getInt("num_tilings", true);
  i_num_rectangles = settings.getInt("num_rectangles", true);   
  i_mem_size_mult = settings.getInt("mem_size_multiplier", true); 
  i_max_obj_velocity = settings.getInt("max_obj_velocity", true); 
  i_max_num_detected_instaces = settings.getInt("max_num_detected_instaces", 
                                                true); 
  i_max_obj_vel_half = i_max_obj_velocity / 2;
  pv_sorted_shape_list = ClassShape::import_shape_list("class_shapes.txt", 
                                                       i_num_classes);

  f_alpha_multiplier = settings.getFloat("alpha_multiplier", true);
  i_plot_class_inst_frq = settings.getInt("plot_class_inst_frq", true);
  b_end_episode_with_reward =settings.getBool("end_episode_with_reward",true);


  // Get the height and width of the screen
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
    
  // calculate the feature vector size
  i_mem_size_2d =   i_num_tilings * pow(i_num_rectangles, 2) * i_mem_size_mult;
  i_mem_size_4d =   i_num_tilings * pow(i_num_rectangles, 4) * i_mem_size_mult; 
  i_num_class_pairs = i_num_classes * (i_num_classes - 1) / 2;
  calc_feature_vec_size();
  cout << "Full Feature-Vector Length: " << i_full_feature_vec_length << endl;
	
  p_sarsa_lambda_solver = RLSarsaLambda::generate_rl_sarsa_lambda_instance(
                                                                           p_osystem, i_full_feature_vec_length, i_num_actions);
	
  // Initilize the feaure-map
  pv_curr_feature_map  = new FeatureMap();
  pv_num_nonzero_in_f = new IntVect();
  for (int i = 0; i < i_num_actions; i++) {
    IntArr feature_vec = IntArr(-1, i_full_feature_vec_length);
    pv_curr_feature_map->push_back(feature_vec);
    pv_num_nonzero_in_f->push_back(0);
  }
    
  // Load the Background matrix
  pm_background_marix = new IntMatrix;
  import_matrix(pm_background_marix, "backgrounds/background_matrix.txt");
    
  // Initilize  the ClassInstace maps
  pv_curr_cls_inst_map = new ClassInstancesMap;
  pv_prev_cls_inst_map = new ClassInstancesMap;
  for (int i = 0; i < i_num_classes; i++) {
    vector<BlobObject> v_curr;
    vector<BlobObject> v_prev;
    pv_curr_cls_inst_map->push_back(v_curr);
    pv_prev_cls_inst_map->push_back(v_prev);
  }
    
  // Initilize the forground matrix
  for (int i = 0; i < i_screen_height; i++) {
    vector<int> row;
    for (int j = 0; j < i_screen_width; j++) {
      row.push_back(0);
    }
    m_forground_matrix.push_back(row);
  }
    
  // Initilize the scale factors
  f_abs_pos_scale_factor_x = (float)i_num_rectangles / (float)i_screen_width;
  f_abs_pos_scale_factor_y = (float)i_num_rectangles / (float)i_screen_height;
  f_rel_pos_scale_factor_x =  (float)i_num_rectangles / 
    (float)(2 * i_screen_width);
  f_rel_pos_scale_factor_y =  (float)i_num_rectangles / 
    (float)(2 * i_screen_height);
  f_vel_scale_factor = (float)i_num_rectangles / (float)i_max_obj_velocity;
    
  // Initlize the temporary vectors used for tile-coding
  pv_tmp_abs_ind = new IntArr(-1, i_num_tilings);
  pv_tmp_rel_ind = new IntArr(-1, i_num_tilings); 
	
}

    
ClassAgent::~ClassAgent() {
  for (unsigned int i = 0; i < pv_sorted_shape_list->size(); i++) {
    delete (*pv_sorted_shape_list)[i];
  }
  delete pv_sorted_shape_list;
  delete pm_background_marix;
  delete pv_curr_cls_inst_map;
  delete pv_prev_cls_inst_map;
  delete pv_curr_feature_map;
  delete pv_num_nonzero_in_f;
  delete p_sarsa_lambda_solver;
  delete pv_tmp_abs_ind;
  delete pv_tmp_rel_ind;
}

/* *********************************************************************
   Calculates the size of the full feature-vector
   ******************************************************************** */
void ClassAgent::calc_feature_vec_size(void) {
  i_full_feature_vec_length = 0;
  if (b_inc_abs_positions) { 
    i_full_feature_vec_length += (	i_num_classes * 
                                        (i_mem_size_2d + i_num_actions));
  }
  i_full_feature_vec_length += (i_num_class_pairs * i_mem_size_2d * 4);
}
        
/* *********************************************************************
   Returns an action from the set of possible actions.
   Runs one step of the Sarsa-Lambda algorithm
   ******************************************************************** */
Action ClassAgent::agent_step(  const IntMatrix* screen_matrix, 
                                const IntVect* console_ram, 
                                int frame_number) {
  Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram,
                                                  frame_number);
  if (special_action != UNDEFINED) {
    return special_action;  // We are resettign or in some sort of delay 
  }
  if (b_end_episode_with_reward && b_reward_on_this_frame) {
    cout << "Ending episode with reward: V(end) = " << f_curr_reward << endl;
    p_sarsa_lambda_solver->episode_end(f_curr_reward, f_curr_reward);
    Action first_act = on_start_of_game();
    return first_act;
  }

  get_class_instances_on_screen();
  calc_object_velocities();
  if ((i_plot_class_inst_frq != 0) &&
      ((i_frame_counter % i_plot_class_inst_frq) == 0)) {
    plot_current_class_instances();
  }

  generate_feature_vec();
  int next_action_ind;
  if (e_episode_status == ACTION_EXPLOR) {
    next_action_ind = p_sarsa_lambda_solver->episode_step(
                                                          pv_curr_feature_map, 
                                                          pv_num_nonzero_in_f,
                                                          f_curr_reward, 
                                                          i_curr_expl_act_index);
  } else {
    next_action_ind = p_sarsa_lambda_solver->episode_step(
                                                          pv_curr_feature_map, 
                                                          pv_num_nonzero_in_f,
                                                          f_curr_reward);
  }
  assert(next_action_ind < i_num_actions);
  return (*(p_game_settings->pv_possible_actions))[next_action_ind];
}

/* *********************************************************************
   This method is called at the beginnig of each game
   ******************************************************************** */
Action ClassAgent::on_start_of_game(void) {
  get_class_instances_on_screen();
  calc_object_velocities();
  generate_feature_vec();
  int next_action_ind;
  if (e_episode_status == ACTION_EXPLOR) {
    next_action_ind = p_sarsa_lambda_solver->episode_start(
                                                           pv_curr_feature_map, pv_num_nonzero_in_f, 
                                                           i_curr_expl_act_index);
  } else {
    next_action_ind = p_sarsa_lambda_solver->episode_start(
                                                           pv_curr_feature_map, pv_num_nonzero_in_f);
  }
  return  (*(p_game_settings->pv_possible_actions))[next_action_ind];
}

/* *********************************************************************
   This method is called when the game ends. 
   ******************************************************************** */
void ClassAgent::on_end_of_game(void) {
  PlayerAgent::on_end_of_game();
  // On the end of the game, we go to a terminating state where
  //  all future rewards are the current reward we recieved
  cout << "V(end) = " << f_curr_reward << endl;
  p_sarsa_lambda_solver->episode_end(f_curr_reward, f_curr_reward);
}
        
/* *********************************************************************
   Generates a feature vector based on the instance of the
   blob objects that are currently on the screen
   The way this works is that we have a tile-coding for each pair of
   classes (e.g. [chicken,. chicken], [chicken, car], [car, car])
   The feature vector consists of these tilings concatinated together.
   For each pair of objects on the screen, we compute a tiling based on
   the classes they belong to, and add  it to the appropriate part of
   the final feature vector.
   Example: if we only had two classes A, and B, the feature vector 
   would be as follows (each bracket is a seperate tile coding)
   part 1, absolute positions: [A.x, A.y] [B.x, B.y]
   part 2, relative position/velocity: [   A.x - B.x, A.y - B.y ] X 4
   depending on whether the objects are moving toward each other in x or y
   axis or not.
   if include_abs_positions is false, we will ignore the first part
   ******************************************************************** */
void ClassAgent::generate_feature_vec(void) {
  for (int a = 0; a < i_num_actions; a++) {
    int start_ind = 0;
    (*pv_num_nonzero_in_f)[a] = 0;
    if (b_inc_abs_positions) {
      // Generate the absolute position part of the vector
      for (int m = 0; m < i_num_classes; m++) {
        int num_instances_in_class = (*pv_curr_cls_inst_map)[m].size();
        if (num_instances_in_class == 0) {
          // No intances of this class found. The first i_num_actions
          // bits of the feature subvector are reserved to indicate 
          // this
          int one_ind = a;
          one_ind += start_ind;
          add_one_index_to_feature_map(one_ind, a);
        }
        start_ind += i_num_actions;
        for(int obj_cntr = 0; 
            obj_cntr < num_instances_in_class; obj_cntr++) {
          BlobObject& obj = (*pv_curr_cls_inst_map)[m][obj_cntr];
          float scaled_x, scaled_y;
          get_scaled_position(obj, scaled_x, scaled_y);
          pf_tmp_float_arr[0] = scaled_x;
          pf_tmp_float_arr[1] = scaled_y;
          GetTiles(pv_tmp_abs_ind, i_num_tilings, i_mem_size_2d,
                   pf_tmp_float_arr, 2, a); 
          (*pv_tmp_abs_ind) += start_ind;
          append_to_feature_vec(pv_tmp_abs_ind, i_num_tilings, a);
        }
        start_ind += i_mem_size_2d;
      }
    }
    // Generate the relative position/velocity part of the vector
    for (int m_a = 0; m_a < i_num_classes; m_a++ ) {
      int num_instances_in_a = (*pv_curr_cls_inst_map)[m_a].size();
      if (num_instances_in_a > i_max_num_detected_instaces) {
        num_instances_in_a = i_max_num_detected_instaces;
      }
      for (int m_b = m_a + 1; m_b < i_num_classes; m_b++) {
        int num_instances_in_b = (*pv_curr_cls_inst_map)[m_b].size();
        if (num_instances_in_b > i_max_num_detected_instaces) {
          num_instances_in_b = i_max_num_detected_instaces;
        }
        for(int obj_cnt_a = 0; 
            obj_cnt_a < num_instances_in_a; obj_cnt_a++) {
          BlobObject& obj_a = (*pv_curr_cls_inst_map)[m_a][obj_cnt_a];
          for(int obj_cnt_b = 0; 
              obj_cnt_b < num_instances_in_b; obj_cnt_b++) {
            BlobObject& obj_b = (*pv_curr_cls_inst_map)[m_b]
              [obj_cnt_b];
            float x_rel_scaled, y_rel_scaled;
            get_scaled_relative_position(obj_a, obj_b, 
                                         x_rel_scaled, y_rel_scaled);
            bool is_approaching_x, is_approaching_y;
            get_binary_relative_velocity(obj_a, obj_b, 
                                         is_approaching_x, is_approaching_y);
            int vel_offset = 0;
            if (is_approaching_x) {
              vel_offset += 1;
            } 
            if (is_approaching_y) {
              vel_offset += 2;
            }
            assert(vel_offset >= 0 && vel_offset <= 3);
            vel_offset = vel_offset * i_mem_size_2d;
            pf_tmp_float_arr[0] = x_rel_scaled;
            pf_tmp_float_arr[1] = y_rel_scaled;
            GetTiles(pv_tmp_rel_ind, i_num_tilings, i_mem_size_2d,
                     pf_tmp_float_arr, 2, a); 
            (*pv_tmp_rel_ind) += (start_ind + vel_offset);
            append_to_feature_vec(pv_tmp_rel_ind, i_num_tilings, a);
          }
        }
        start_ind += (4 * i_mem_size_2d);
      }
    }
    assert (start_ind == i_full_feature_vec_length);
  }
}

/* *********************************************************************
   For each blob class defined in discovered_classes, returns a
   list of instance objects currently on the screen

   ******************************************************************** */
void ClassAgent::get_class_instances_on_screen() { 
  swap_curr_and_prev_class_instances(); // prev_class_inst = curr_class_inst
  extract_forground();
  int num_forg_pix = v_forground_x_ind.size();    
  for (   int shape_counter = 0; shape_counter < pv_sorted_shape_list->size(); 
          shape_counter++) {
    ClassShape* curr_shape = (*pv_sorted_shape_list)[shape_counter];
    IntMatrix* shape_matrix = curr_shape->pm_shape;
            
    int row_range_start = 0;
    int row_range_end   = curr_shape->i_height;
    int col_range_start = 0 - curr_shape->i_first_one_ind;
    int col_range_end   = curr_shape->i_width - curr_shape->i_first_one_ind;
    int half_height = curr_shape->i_height / 2;
    int half_width  = curr_shape->i_width / 2;
    for (int ind = 0; ind < num_forg_pix; ind++) {
      int y = v_forground_y_ind[ind];
      int x = v_forground_x_ind[ind];
      if (y + row_range_start < 0             || 
          y + row_range_end   > i_screen_height || 
          x + col_range_start < 0             || 
          x + col_range_end   > i_screen_width) {
        continue;
      }
      int shape_matches = 1;
      for (int row_ind = 0; row_ind < curr_shape->i_height; row_ind++) {
        for (int col_ind = 0; col_ind < curr_shape->i_width; col_ind++) {
          if (m_forground_matrix[y + row_ind + row_range_start]
              [x + col_ind + col_range_start] !=
              (*shape_matrix)[row_ind][col_ind] ) {
            shape_matches = 0;
            break;
          }
        }
        if (!shape_matches) {
          break;
        }
      }
      if (shape_matches) {
        // we have found an instance of this class
        int center_x = x + col_range_start + half_width;
        int center_y = y + row_range_start + half_height;
        BlobObject new_obj(center_x, center_y);
        new_obj.i_instance_of_class = curr_shape->i_instance_of_class;
        (*pv_curr_cls_inst_map)[curr_shape->i_instance_of_class].push_back(new_obj);
        // make sure we don't match this to a smaller class
        for (int row_ind = 0; row_ind < curr_shape->i_height; row_ind++) {
          for (int col_ind = 0; col_ind < curr_shape->i_width; col_ind++){
            m_forground_matrix[y + row_ind + row_range_start]
              [x + col_ind + col_range_start] = 0;
          }
        }
      }
    }
  }
}

/* *********************************************************************
   Calculates the velocity of each object detected on screen.
   We do this by looking at each class instance in the current
   screen, and looking at instances of the same class in previous
   screen and finding the closest object. The displacement will then
   indicate the object velocity
	
   NOTE: We should be looking at object similarities here too.
   ******************************************************************** */
void ClassAgent::calc_object_velocities(void) {
  for (int class_cntr = 0; class_cntr < i_num_classes; class_cntr++) {
    for(int obj_cntr = 0; 
        obj_cntr < (*pv_curr_cls_inst_map)[class_cntr].size();
        obj_cntr++) {
      BlobObject& curr_obj = (*pv_curr_cls_inst_map)[class_cntr][obj_cntr];
      // Now, find the closest object to curr-obj in the previous frame
      int closest_distance_sq = 999;
      for (   int prev_obj_cntr = 0; 
              prev_obj_cntr < (*pv_prev_cls_inst_map)[class_cntr].size();
              prev_obj_cntr++ ) {
        BlobObject prev_obj = (*pv_prev_cls_inst_map)[class_cntr]
          [prev_obj_cntr];
        int dist = curr_obj.calc_distance_max_xy(&prev_obj);
        assert (dist >= 0);
        if (dist <= i_max_obj_velocity &&
            dist < closest_distance_sq) {
          curr_obj.i_velocity_x = curr_obj.i_center_x - 
            prev_obj.i_center_x;
          curr_obj.i_velocity_y = curr_obj.i_center_y - 
            prev_obj.i_center_y;
          closest_distance_sq = dist;
        }
      }
    }
  }
}


/* *********************************************************************
   Sets pv_curr_cls_inst_map to pv_prev_cls_inst_map
   ******************************************************************** */
void ClassAgent::swap_curr_and_prev_class_instances() {
  delete pv_prev_cls_inst_map;
  pv_prev_cls_inst_map = pv_curr_cls_inst_map;
  pv_curr_cls_inst_map = new ClassInstancesMap;
  for (int i = 0; i < i_num_classes; i++) {
    vector<BlobObject> v_curr;
    pv_curr_cls_inst_map->push_back(v_curr);
  }
}   

/* *********************************************************************
   Extracts the foreground by subtracting the background matrix from
   the screen matrix, and fills the m_forground_matrix, 
   v_forground_x_ind, and v_forground_y_ind vectors
   ******************************************************************** */
void ClassAgent::extract_forground() {
  v_forground_x_ind.clear();
  v_forground_y_ind.clear();
  for (int i = 0; i < i_screen_height; i++) {
    for(int j = 0; j < i_screen_width; j++) {
      if ((*pm_curr_screen_matrix)[i][j] != (*pm_background_marix)[i][j]){
        v_forground_y_ind.push_back(i);
        v_forground_x_ind.push_back(j);
        m_forground_matrix[i][j] = 1;
      } else {
        m_forground_matrix[i][j] = 0;
      }
    }
  }
}               

/* *********************************************************************
   Returns the scaled position of the given blob object, so that its
   x,y values are between 0 and self.num_rectangles
   ******************************************************************** */
void ClassAgent::get_scaled_position(   BlobObject& blob_obj, 
                                        float& x_scaled, float& y_scaled) {
  x_scaled = blob_obj.i_center_x * f_abs_pos_scale_factor_x;
  y_scaled = blob_obj.i_center_y * f_abs_pos_scale_factor_y;
  assert (x_scaled >= 0 && x_scaled <= i_num_rectangles && 
          y_scaled >= 0 && y_scaled <= i_num_rectangles);
}

/* *********************************************************************
   Returns the scaled relative position of blob object a in respect to
   blob object b
   ******************************************************************** */
void ClassAgent::get_scaled_relative_position(  BlobObject& obj_a, 
                                                BlobObject& obj_b, float& x_scaled, float& y_scaled) {
  int rel_x = obj_a.i_center_x - obj_b.i_center_x;
  int rel_y = obj_a.i_center_y - obj_b.i_center_y;
  x_scaled = (float)(rel_x + i_screen_width) * f_rel_pos_scale_factor_x;
  y_scaled = (float)(rel_y + i_screen_height) * f_rel_pos_scale_factor_y;
  assert (x_scaled >= 0 && x_scaled <= i_num_rectangles && 
          y_scaled >= 0 && y_scaled <= i_num_rectangles);
}

/* *********************************************************************
   Returns the scaled relative velocity of blob object a in respect to
   blob object b
   ******************************************************************** */
void ClassAgent::get_scaled_relative_velocity(BlobObject& obj_a, 
                                              BlobObject& obj_b,  float& rel_vel_x_scaled, float& rel_vel_y_scaled) {
  int rel_vel_x = obj_a.i_velocity_x - obj_b.i_velocity_x;
  int rel_vel_y = obj_a.i_velocity_y - obj_b.i_velocity_y;
  // HACK: Make sure relative velocity stays within
  // [-max_obj_velocity / 2, max_obj_velocity / 2]
  // I do this, so that vel=0 and vel=1 don't end up in the same tile,
  // with relatively reasonable num_rectangles
  bound(rel_vel_x, -i_max_obj_vel_half, i_max_obj_vel_half);
  bound(rel_vel_y, -i_max_obj_vel_half, i_max_obj_vel_half);
  rel_vel_x_scaled =  (float)(rel_vel_x + i_max_obj_vel_half) * 
    f_vel_scale_factor;
  rel_vel_y_scaled = (float)(rel_vel_y + i_max_obj_vel_half) * 
    f_vel_scale_factor;
  assert (rel_vel_x_scaled >= 0 && rel_vel_x_scaled <= i_num_rectangles && 
          rel_vel_y_scaled >= 0 && rel_vel_y_scaled <= i_num_rectangles);
}

/* *********************************************************************
   Returns two binary values, indicating weather the two objects are
   moving toward each other in x and y axis
   ******************************************************************** */
void ClassAgent::get_binary_relative_velocity(BlobObject& obj_a, BlobObject& obj_b, 
                                              bool& is_approaching_x, bool& is_approaching_y) {
  int rel_vel_x, rel_vel_y;
  if (obj_a.i_center_x < obj_b.i_center_x) {
    // A is in the left of B
    rel_vel_x = obj_b.i_velocity_x - obj_a.i_velocity_x;
  } else {
    // B is in the left of A
    rel_vel_x = obj_a.i_velocity_x - obj_b.i_velocity_x;
  }
  if (obj_a.i_center_y < obj_b.i_center_y) {
    // A is on Top of B
    rel_vel_y = obj_b.i_velocity_y - obj_a.i_velocity_y;
  } else {
    // B is on Top of A
    rel_vel_y = obj_a.i_velocity_y - obj_b.i_velocity_y;
  }
  if (rel_vel_x < 0 ) {
    is_approaching_x = true;
  } else {
    is_approaching_x = false;
  }
  if (rel_vel_y < 0 ) {
    is_approaching_y = true;
  } else {
    is_approaching_y = false;
  }
}

/* *********************************************************************
   Appends a new subvector of length size to feature_map[a]
   ******************************************************************** */
void ClassAgent::append_to_feature_vec(IntArr* new_vec, int size, int a) {
  for(int i = 0; i < size; i++) {
    int curr_ind = (*pv_num_nonzero_in_f)[a];
    (*pv_curr_feature_map)[a][curr_ind] = (*new_vec)[i];
    (*pv_num_nonzero_in_f)[a] = (*pv_num_nonzero_in_f)[a] + 1;
  }
}

/* *********************************************************************
   Plots the current class instances on the screen.
   The istances of each class are plotted with a different symbol
   ******************************************************************** */
void ClassAgent::plot_current_class_instances(void) {
  // Make a copy of curr_screen (we are gonna change it)
  IntMatrix* pv_screen_copy = new IntMatrix(*pm_curr_screen_matrix);
  // For each class member, put a circle mark on the screen
  int color_index = 300;  // first 256 colors are for atari pallete
  for (int class_counter = 0; class_counter < i_num_classes;class_counter++) {
    color_index++;
    for (   int inst_counter = 0; 
            inst_counter < (*pv_curr_cls_inst_map)[class_counter].size();
            inst_counter++ ) {
      BlobObject obj =(*pv_curr_cls_inst_map)[class_counter][inst_counter];
      // plot a rectangle for the object
      for (int x = obj.i_center_x - 2; x <= obj.i_center_x + 2; x++) {
        for (int y = obj.i_center_y - 2; y <= obj.i_center_y + 2; y++) {
          if (x >= 0 && x < i_screen_width &&
              y >= 0 && y < i_screen_height) {
            (*pv_screen_copy)[y][x] = color_index;
          }
        }
      }
      // plot a line for velocity
      draw_line(  obj.i_center_x, obj.i_center_y, 
                  obj.i_center_x + obj.i_velocity_x, 
                  obj.i_center_y + obj.i_velocity_y, 
                  pv_screen_copy);
    }
  }
  // Save the matrix as PNG
  ostringstream filename;
  char buffer [50];
  sprintf (buffer, "%09lld", i_frame_counter);
  filename << "class_instances__frame_" << buffer << ".png";
  p_osystem->p_export_screen->save_png(pv_screen_copy, filename.str());
  delete pv_screen_copy;
}


/* *********************************************************************
   Draws a line on the screen matrix (used for plotting the velocity)
   ******************************************************************** */
void ClassAgent::draw_line( int x1, int y1, 
                            int x2, int y2, IntMatrix* pv_screen, 
                            int color_ind){
  int xdiff = (x2 - x1);
  int ydiff = (y2 - y1);

  if(xdiff == 0 && ydiff == 0) {
    (*pv_screen)[y1][x1] = color_ind;
    return;
  }

  if(abs(xdiff) > abs(ydiff)) {
    int xmin, xmax;

    // set xmin to the lower x value given
    // and xmax to the higher value
    if (x1 < x2) {
      xmin = x1;
      xmax = x2;
    } else {
      xmin = x2;
      xmax = x1;
    }

    // draw line in terms of y slope
    float slope = (float)ydiff / (float)xdiff;
    for(int x = xmin; x <= xmax; x += 1) {
      int y = y1 + (int)((x - x1) * slope);
      if (x >= 0 && x < i_screen_width &&
          y >= 0 && y < i_screen_height) {
        (*pv_screen)[y][x] = color_ind;
      }
    }
  } else {
    int ymin, ymax;

    // set ymin to the lower y value given
    // and ymax to the higher value
    if (y1 < y2) {
      ymin = y1;
      ymax = y2;
    } else {
      ymin = y2;
      ymax = y1;
    }

    // draw line in terms of x slope
    float slope = (float)xdiff / (float)ydiff;
    for(int y = ymin; y <= ymax; y += 1) {
      int x = x1 + (int)((y - y1) * slope);
      if (x >= 0 && x < i_screen_width &&
          y >= 0 && y < i_screen_height) {
        (*pv_screen)[y][x] = color_ind;
      }
    }
  }
}

