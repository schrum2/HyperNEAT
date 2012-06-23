/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  class_agent.h
 *
 *  The implementation of the ClassAgent class, which is a generic player 
 *  agent that learns to play, based on detecting instances of blob classes
 *  on the screen, and using tile coding on their relative position and velocity
 **************************************************************************** */

#ifndef CLASS_AGENT_H
#define CLASS_AGENT_H

#include <vector>
#include "common_constants.h"
#include "player_agent.h"
#include "blob_object.h"
#include "rl_sarsa_lambda.h"
#include "class_shape.h"

typedef vector< vector<BlobObject> > ClassInstancesMap;   

class ClassAgent : public PlayerAgent {
  /* *************************************************************************
     This isa generic player agent that learns to play, based on detecting 
     instances of blob classes on the screen, and using tile coding on their 
     relative position and velocity        
        
     Instance variabls:
     - b_inc_abs_positions   When True, we will include the absolute
     positions of class instances in the feature
     vector
     - i_num_tilings         Number of tilings
     - i_num_rectangles      Number fo rectangles 
     - i_mem_size_mult       Memory size multiplier
     - i_mem_size_2d         memory size for 2D tiling
     - i_mem_size_4d         memory size for 4D tiling
     - alpha_multiplier  alpha = 1.0 / 
     (num_ones_in_vector * rl_params.alpha_multiplier)
     - pv_curr_feature_map   pv_curr_feature_map[a] is the feature-vector 
     for action a.
     - pv_num_nonzero_in_f   Number of non-zero values in each feature-vector 
     - p_sarsa_lambda_solver Pointer to the SARSA-Lambda solver object
     - i_full_feature_vec_length     Number of bits in the *full* feature
     vector. Note that the feature-map that we 
     generate only holds the one-indecies of this 
     vector
     - pm_background_marix   Matrix contaiing the background color inds
     - i_num_classes         Number of blob classes
     - i_num_class_pairs     How many class pairs (c1, c2) there is
     - pv_sorted_shape_list  A vector of class shapes, sorted by size.
     Used to detect class instances on the screen
     - pv_curr_cls_inst_map  pv_curr_cls_inst_map[c] is a vector of 
     BlobObjects, indicating the instance of class
     c in the current screen
     - pv_prev_cls_inst_map  pv_prev_cls_inst_map[c] is a vector of 
     BlobObjects, indicating the instance of class
     c in the previous screen
     - m_forground_matrix    m_forground_matrix[i][j] = 1 if the (i,j) pixel
     is in the forground, 0 if it is in background
     - v_forground_y_ind     Y indecies of the forground pixels
     - v_forground_x_ind     X indecies of the forground pixels
     - i_screen_height       Height of the screen
     - i_screen_width        Width of the screen
     - f_abs_pos_scale_factor_x  Precomputed factors (_x,_y) for scaling
     - f_abs_pos_scale_factor_y  absolute-positions
     - f_rel_pos_scale_factor_x  Precomputed factors (_x,_y) for scaling
     - f_rel_pos_scale_factor_y  relative-positions
     - f_vel_scale_factor      Precomputed factor for scaling velocities
     - pv_tmp_abs_ind        Temporary vector used to get absolute indecies
     from the tile-coding code
     - pv_tmp_rel_ind        Temporary vector used to get reltive indecies
     from the tile-coding code
     - pf_tmp_float_arr      Temporary float array, used to send float values 
     to the tile-coding code
     - i_plot_class_inst_frq	How often to plot the class instances (0 means 
     never)
     - b_end_episode_with_reward  When true, episode ends after each reward.
     - i_max_obj_velocity    Maximum velocity (pixel/sec) of objects on 
     screen
     - i_max_obj_vel_half	i_max_obj_velocity / 2 
     - i_max_num_detected_instaces	Maximum number of instances that will be
     detected from each class
     ************************************************************************* */

 public:
  ClassAgent(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~ClassAgent();
        
  /* *********************************************************************
     Returns an action from the set of possible actions.
     Runs one step of the Sarsa-Lambda algorithm
     ******************************************************************** */
  virtual Action agent_step(  const IntMatrix* screen_matrix, 
                              const IntVect* console_ram, 
                              int frame_number);
        
  /* *********************************************************************
     This method is called when the game ends. 
     ******************************************************************** */
  virtual void on_end_of_game(void);

  /* *********************************************************************
     This method is called at the beginnig of each game
     ******************************************************************** */
  virtual Action on_start_of_game(void);

 protected:
  /* *********************************************************************
     Calculates the size of the full feature-vector
     ******************************************************************** */
  virtual void calc_feature_vec_size(void);
		
  /* *********************************************************************
     Generates a feature vector based on the instance of the
     blob objects that are currently on the screen
     The way this works is that we have a tile-coding for each pair of
     classes (e.g. [chicke,. chicken], [chicken, car], [car, car])
     The feature vector consists of these tilings concatinated together.
     For each pair of objects on the screen, we compute a tiling based on
     the classes they belong to, and add  it to the appropriate part of
     the final feature vector.
     Example: if we only had two classes A, and B, the feature vector 
     would be as follows (each bracket is a seperate tile coding)
     part 1, absolute positions: [A.x, A.y] [B.x, B.y]
     part 2, relative position/velocity: [   A.x - B.x, A.y - B.y,
     A.dx - B.dx, A.dy - B.dy]
     if include_abs_positions is false, we will ignore the first part
     This version generates a full vector (not just the one indecies)
     it should be only used for debugging the sparse version
     ******************************************************************** */
  virtual void generate_feature_vec(void);

  /* *********************************************************************
     For each blob class defined in discovered_classes, returns a
     list of instance objects currently on the screen
     Note: For each class, we will detect at most max_num_instaces
     instances. 
     ******************************************************************** */
  void get_class_instances_on_screen(void);
        
  /* *********************************************************************
     Calculates the velocity of each object detected on screen.
     We do this by looking at each class instance in the current
     screen, and looking at instances of the same class in previous
     screen and finding the closest object. The displacement will then
     indicate the object velocity
     ******************************************************************** */
  void calc_object_velocities(void);
        
  /* *********************************************************************
     Sets pv_curr_cls_inst_map to pv_prev_cls_inst_map
     ******************************************************************** */
  void swap_curr_and_prev_class_instances();
        
  /* *********************************************************************
     Extracts the foreground by subtracting the background matrix from
     the screen matrix, and fills the m_forground_matrix, 
     v_forground_x_ind, and v_forground_y_ind vectors
     ******************************************************************** */
  void extract_forground();

  /* *********************************************************************
     Plots the current class instances on the screen.
     The istances of each class are plotted with a different symbol
     ******************************************************************** */
  void plot_current_class_instances(void);
        
  /* *********************************************************************
     Draws a line on the screen matrix (used for plotting the velocity)
     ******************************************************************** */
  void draw_line(int x1, int y1, int x2, int y2, IntMatrix* pv_screen, 
                 int color_ind = 256);

  /* *********************************************************************
     Returns the scaled position of the given blob object, so that its
     x,y values are between 0 and self.num_rectangles
     ******************************************************************** */
  void get_scaled_position(BlobObject& blob_obj, 
                           float& x_scaled, float& y_scaled);

  /* *********************************************************************
     Returns the scaled relative position of blob object a in respect to
     blob object b
     ******************************************************************** */
  void get_scaled_relative_position(BlobObject& obj_a, BlobObject& obj_b,
                                    float& x_scaled, float& y_scaled);

  /* *********************************************************************
     Returns the scaled relative velocity of blob object a in respect to
     blob object b
     ******************************************************************** */
  void get_scaled_relative_velocity(BlobObject& obj_a, BlobObject& obj_b, 
                                    float& rel_vel_x_scaled, 
                                    float& rel_vel_y_scaled);

  /* *********************************************************************
     Returns two binary values, indicating weather the two objects are
     moving toward each other in x and y axis
     ******************************************************************** */
  void get_binary_relative_velocity(BlobObject& obj_a, BlobObject& obj_b, 
                                    bool& is_approaching_x, 
                                    bool& is_approaching_y);

        
  /* *********************************************************************
     Appends a new subvector of length size to feature_map[a]
     ******************************************************************** */
  void append_to_feature_vec(IntArr* new_vec, int size, int a);

  /* *********************************************************************
     Adds an indecie of a one value to the feature map[a]
     ******************************************************************** */
  inline void add_one_index_to_feature_map(int one_index, int a) {
    (*pv_curr_feature_map)[a][(*pv_num_nonzero_in_f)[a]] = one_index ;
    (*pv_num_nonzero_in_f)[a] = (*pv_num_nonzero_in_f)[a] + 1;            
  }
		        
		
  bool b_inc_abs_positions;                   
  int i_num_tilings;      
  int i_num_rectangles;   
  int i_mem_size_mult;    
  int i_mem_size_2d;      
  int i_mem_size_4d;      
  FeatureMap* pv_curr_feature_map;
  IntVect* pv_num_nonzero_in_f;
  RLSarsaLambda* p_sarsa_lambda_solver;
  int i_full_feature_vec_length;
  float f_alpha_multiplier;
  int i_num_classes;
  int i_num_class_pairs;
  IntMatrix* pm_background_marix;
  ShapeList* pv_sorted_shape_list;
  ClassInstancesMap* pv_curr_cls_inst_map;
  ClassInstancesMap* pv_prev_cls_inst_map;
  IntMatrix m_forground_matrix;
  IntVect v_forground_y_ind;
  IntVect v_forground_x_ind;
  int i_screen_height;
  int i_screen_width;
  float f_abs_pos_scale_factor_x;
  float f_abs_pos_scale_factor_y;
  float f_rel_pos_scale_factor_x;
  float f_rel_pos_scale_factor_y;
  float f_vel_scale_factor;
  IntArr* pv_tmp_abs_ind;
  IntArr* pv_tmp_rel_ind;
  float pf_tmp_float_arr[MAX_NUM_TILING_VARS];
  int i_plot_class_inst_frq;
  bool b_end_episode_with_reward;
  int i_max_obj_velocity;
  int i_max_obj_vel_half;
  int i_max_num_detected_instaces;
};


#endif
