/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  grid_screen_agent.h
 *
 *  The implementation of the GridScrAgent class, which is a generic player 
 *  agent that learns to play by directly using the screen 
 **************************************************************************** */

#include <sstream>
#include "export_tools.h"
#include "game_controller.h"
#include "grid_screen_agent.h"

        
GridScrAgent::GridScrAgent(GameSettings* _game_settings, OSystem* _osystem) :
  PlayerAgent(_game_settings, _osystem) {
  // Get the height and width of the screen
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
  cout << "Screen Width:" << i_screen_width << "Screen Height:" << i_screen_height << endl;
  printf("%d\n",i_screen_width);
  i_num_colors = 8;
  b_do_subtract_bg = p_osystem->settings().getBool("do_subtract_background",
                                                   true);
  i_num_block_per_row = p_osystem->settings().getInt("num_block_per_row", 
                                                     true);
  i_num_block_per_col = p_osystem->settings().getInt("num_block_per_col", 
                                                     true);
  if ( (i_screen_width % i_num_block_per_col != 0) ||
       (i_screen_height % i_num_block_per_row != 0) ) {
    cerr << "WARNING: Invalid i_num_block_per_row or i_num_block_per_col value."
         << "Screen-Width should be divisible by i_num_block_per_col. "
         << "Screen-Height should be divisible by i_num_block_per_row." 
         << endl;
    exit(-1);
  }
  i_block_height = i_screen_height / i_num_block_per_row;
  i_block_width = i_screen_width / i_num_block_per_col;
  i_blocks_bits_length = i_num_block_per_row*i_num_block_per_col*i_num_colors;
	
  i_base_length = i_blocks_bits_length +      // feature-vec length for one action
    (i_blocks_bits_length * (i_blocks_bits_length / 2) - 
     (i_blocks_bits_length / 2));

  i_full_feature_vec_length = i_base_length * i_num_actions;
  cout << "Full Feature-Vector Length: " << i_full_feature_vec_length << endl;
  p_sarsa_lambda_solver = RLSarsaLambda::generate_rl_sarsa_lambda_instance(p_osystem, i_full_feature_vec_length, i_num_actions);
  pv_curr_feature_map  = new FeatureMap();
  pv_num_nonzero_in_f = new IntVect();
  for (int i = 0; i < i_num_actions; i++) {
    IntArr feature_vec = IntArr(-1, i_base_length);
    pv_curr_feature_map->push_back(feature_vec);
    pv_num_nonzero_in_f->push_back(0);
  }
  pv_tmp_fv_first_part = new IntArr(-1, i_blocks_bits_length);
  pv_tmp_color_bits = new IntVect;
  for (int c = 0; c < i_num_colors; c++) {
    pv_tmp_color_bits->push_back(0);
  }
  pv_subvector_positions = new IntVect; // this is for optimization only
  for (int a = 0; a < i_num_actions; a++) {
    pv_subvector_positions->push_back(a * i_base_length);
  }

  b_end_episode_with_reward = p_osystem->settings().getBool(
                                                            "end_episode_with_reward", true);
  b_plot_dirscr_grids = p_osystem->settings().getBool("plot_gridscr_grids", 
                                                      true);
  if (b_do_subtract_bg) {
    // Load the background matrix
    pm_background_matrix = new IntMatrix;
    import_matrix(pm_background_matrix, "backgrounds/background_matrix.txt");
  } else {
    pm_background_matrix = NULL;
  }
}

GridScrAgent::~GridScrAgent() {
  delete p_sarsa_lambda_solver;
  delete pv_curr_feature_map;
  delete pv_num_nonzero_in_f;
  delete pv_tmp_fv_first_part;
  delete pv_tmp_color_bits;
  delete pv_subvector_positions;
  if (pm_background_matrix) {
    delete pm_background_matrix;
  }
}

/* *********************************************************************
   Returns a random action from the set of possible actions
   ******************************************************************** */
Action GridScrAgent::agent_step(  const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram, int frame_number) {
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
  generate_feature_vec();
  if (b_plot_dirscr_grids) {
    plot_extracted_grid();
  }
	
  int next_action_ind;
  if (e_episode_status == ACTION_EXPLOR) {
    next_action_ind = p_sarsa_lambda_solver->episode_step(
                                                          pv_curr_feature_map, 
                                                          pv_num_nonzero_in_f,
                                                          f_curr_reward, 
                                                          i_curr_expl_act_index);
    assert(next_action_ind == i_curr_expl_act_index);
  } else {
    next_action_ind = p_sarsa_lambda_solver->episode_step(
                                                          pv_curr_feature_map, 
                                                          pv_num_nonzero_in_f,
                                                          f_curr_reward);
  }
  return (*(p_game_settings->pv_possible_actions))[next_action_ind];	
}
						
/* *********************************************************************
   This method is called at the beginnig of each game
   ******************************************************************** */
Action GridScrAgent::on_start_of_game(void) {
  generate_feature_vec();
  assert((*pv_num_nonzero_in_f)[0] > 0);
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
void GridScrAgent::on_end_of_game(void) {
  PlayerAgent::on_end_of_game();
  cout << "V(end) = " << f_curr_reward << endl;
  p_sarsa_lambda_solver->episode_end(f_curr_reward, f_curr_reward);
}

void GridScrAgent::generate_feature_vec(void) {
  for (int a = 0; a < i_num_actions; a++) {
    (*pv_num_nonzero_in_f)[a] = 0;
  }
  int full_vect_index = 0;
  int i, j, c, bit_val;
  // Get the color-bits for all blocks into our temp array,
  // Also, generate the first part of the feature-vector
  for (c = 0; c < i_num_colors; c++) {
    (*pv_tmp_color_bits)[c] = 0;
  } 
  for (i = 0; i < i_num_block_per_row; i++) {
    for (j = 0; j < i_num_block_per_col; j++) {	
      get_color_ind_from_block(i, j, pv_tmp_color_bits);
      for (c = 0; c < i_num_colors; c++) {
        bit_val = (*pv_tmp_color_bits)[c];
        (*pv_tmp_fv_first_part)[full_vect_index] = bit_val;
        if (bit_val == 1) {
          add_one_index_to_feature_map(full_vect_index);
        }
        full_vect_index++;
      }
    }
  }
  // Now we'll get the crossproduct of all the bits
  for (int i = 0; i < i_blocks_bits_length; i++) {
    //      Possible major oprimization:
    //        if ((*pv_tmp_fv_first_part)[i]  == 0) {
    //            full_vect_index += (i_ram_bits_length - (i + 1));
    //            continue;
    //        }
    for (int j = i + 1; j < i_blocks_bits_length; j++) {
      if ((*pv_tmp_fv_first_part)[i]  == 1 &&
          (*pv_tmp_fv_first_part)[j] == 1) { 
        add_one_index_to_feature_map(full_vect_index);
      }
      full_vect_index++;
    }
  }
  assert(full_vect_index == i_base_length);
}




/* *********************************************************************
   Given the [i,j] indecies of a block, generates a integer vector 
   color_inds, such that color_inds[c] = 1 if the color c exists in 
   the block.
   ******************************************************************** */
void GridScrAgent::get_color_ind_from_block(int i, int j, IntVect* color_bits) {
  // reset the given vector all to zero
  for (int c = 0; c < i_num_colors; c++) {
    (*color_bits)[c] = 0;
  }
  assert (i >= 0 && i < i_num_block_per_row);
  assert (j >= 0 && j < i_num_block_per_col);
  int img_i = i * i_block_height;
  int img_j = j * i_block_width;
  for (int block_row = 0; block_row < i_block_height; block_row++) {
    for (int block_col = 0; block_col < i_block_width; block_col++) {
      int y = img_i + block_row;
      int x = img_j + block_col;
      assert (y >= 0 && y < i_screen_height);
      assert (x >= 0 && x < i_screen_width);
      int color_ind = (*pm_curr_screen_matrix)[y][x];
      if (b_do_subtract_bg && 
          color_ind == (*pm_background_matrix)[y][x] ) {
        continue;	// background pixel
      }
      assert(color_ind >= 0 && color_ind < 256);
      assert(color_ind % 2 == 0);
      int color_bit = pi_eight_bit_pallete[color_ind];
      (*color_bits)[color_bit] = 1;
    }
  }
}


/* *********************************************************************
   Plots the extracted grid. Mostly used for debugging
   ******************************************************************** */
void GridScrAgent::plot_extracted_grid() {
  int box_len = 4;	// Eaxh block will be plotted in 5x5 pixels
  // Initialize the plot matrix
  IntMatrix* grid_plot = new IntMatrix; 
  int plot_rows = (i_num_block_per_row * box_len) + i_num_block_per_row + 1;
  int plot_cols = (i_num_block_per_col * box_len) + i_num_block_per_col + 1;
  for (int i = 0; i < plot_rows; i++) {
    IntVect row;
    for (int j = 0; j < plot_cols; j++) {
      row.push_back(256 + BLACK_COLOR_IND);
    }
    grid_plot->push_back(row);
  }
  // Draw the white borders
  int row_num = 0;
  for (int i = 0; i <= i_num_block_per_row; i++) {
    for (int col_num = 0; col_num < plot_cols; col_num++) {
      (*grid_plot)[row_num][col_num] = 256 + WHITE_COLOR_IND;
    }
    row_num += (box_len + 1);
  }
  int col_num = 0;
  for (int j = 0; j <= i_num_block_per_col; j++) {
    for (int row_num = 0; row_num < plot_rows; row_num++) {
      (*grid_plot)[row_num][col_num] = 256 + WHITE_COLOR_IND;
    }
    col_num += (box_len + 1);
  }
	
  // Fill-in the grids with color
  int curr_row = 1;
  IntVect block_colors;
  for (int i = 0; i < i_num_block_per_row; i++) {
    int curr_col = 1;
    for (int j = 0; j < i_num_block_per_col; j++) {	
      // 1- Get the colors in this block
      get_color_ind_from_block(i, j, pv_tmp_color_bits);
      block_colors.clear();
      for (int c = 0; c < i_num_colors; c++) {
        if ((*pv_tmp_color_bits)[c]) {
          block_colors.push_back(256 + SECAM_COLOR_IND + c);
        }
      }
      // 2- plot a 4x4 box containing all these colors, or leave it
      //	  black if there are no colors
      if (block_colors.size() > 0) {
        int curr_color_ind = 0;
        for (int box_i = 0; box_i < box_len; box_i++) {
          for (int box_j = 0; box_j < box_len; box_j++) {
            (*grid_plot)[curr_row+box_i][curr_col+box_j] =
              block_colors[curr_color_ind];
            curr_color_ind++;
            if (curr_color_ind == block_colors.size()) {
              curr_color_ind = 0;
            }
          }
        }

      } 
      curr_col += (box_len + 1);
    }
    curr_row += (box_len + 1);
  }
	
  // Plot the matrix
  ostringstream filename;
  char buffer [50];
  sprintf (buffer, "%09lld", i_frame_counter);
  filename << "extracted_grid__frame_" << buffer << ".png";
  p_osystem->p_export_screen->export_any_matrix(grid_plot, filename.str());
  delete grid_plot;
}


// A map from color id's to a number between 0, 7
uInt32 GridScrAgent::pi_eight_bit_pallete [256] = {
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0
};
