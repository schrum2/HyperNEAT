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

#ifndef DIRECT_SCREEN_AGENT_H
#define DIRECT_SCREEN_AGENT_H

#include "common_constants.h"
#include "player_agent.h"
#include "rl_sarsa_lambda.h"

class GridScrAgent : public PlayerAgent {
  /* *************************************************************************
     This isa generic player agent that learns to play by directly using 
     the screen         
        
     Instance variabls:
     - pv_curr_feature_map   pv_curr_feature_map[a] is the feature-vector 
     for action a.
     - pv_num_nonzero_in_f   Number of non-zero values in each feature-vector 
     - pv_tmp_fv_first_part  An array containing the first part of the 
     feature-vector (this is only used internally)
     - pv_tmp_color_bits		Temp array that  keeps color-bits for one block
     - p_sarsa_lambda_solver Pointer to the SARSA-Lambda solver object
     - pm_background_matrix	Background matrix
     - f_alpha_multiplier	alpha = 1.0 / 
     (num_ones_in_vector * f_alpha_multiplier)
     - i_num_block_per_row	How many blocks we'll have per row
     - i_num_block_per_col	How many blocks we'll have per column
     - i_num_colors			Number of colors for each block (8 for SECAM)
     - i_blocks_bits_length	(i_num_block_per_row * i_num_block_per_col * 
     i_num_colors) bits 
     - i_base_length         (i_num_block_per_row * i_num_block_per_col * 
     i_num_colors) bits, plus the cross-product
     of these bits. The length of a full 
     feature-vector will be :
     i_num_actions * i_base_length
     - i_full_feature_vec_length     Number of bits in the *full* feature
     vector. Note that the feature-map that we 
     generate only holds the one-indecies of this 
     vector
     - pv_subvector_positions This is only for optimization:
     pv_subvector_positions[a] = a * i_base_length
     - b_end_episode_with_reward  When true, episode ends after each reward.
     - pi_eight_bit_pallete	A map from color id's to a number between 0, 7
     - i_block_height		Number of pixels in the height of each block
     - i_block_width			Number of pixels in the width of each block
     - b_do_subtract_bg		When true we will subtract the background 
     before generating the feature-vector
     - b_plot_dirscr_grids	When true, we will plot the grid extracted for 
     the screen by direc-screen agent
     ************************************************************************* */

 public:
        
  GridScrAgent(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~GridScrAgent();
        
  /* *********************************************************************
     Returns the best action from the set of possible actions
     ******************************************************************** */
  virtual Action agent_step(  const IntMatrix* screen_matrix, 
                              const IntVect* console_ram, 
                              int frame_number);
        
  /* *********************************************************************
     This method is called when the game ends. 
     ******************************************************************** */
  virtual void on_end_of_game(void);
        
 protected:

  /* *********************************************************************
     Generates a feature vector from the content of the screen.
     The screen is divided into a number of blocks (e.g. 10x10). For each
     block, we will create a 8bit vector, determining whether each of the
     8 colors are present. This will result in a 8 * 100 = 800b subvector.
     We also generate the cross-products of every two bits,
     (800 * 800 / 2 - 400 = 319600)
     Since we are required to return a feature_map 
     (i.e. a seperate feature vector for each action), what we do next is 
     we generate a vector of size base_size * num_actions which is all 
     zero, except the portion dedicated for action a.
     ******************************************************************** */
  virtual void generate_feature_vec(void);

  /* *********************************************************************
     Adds an indecie of a one value to the feature map
     ******************************************************************** */
  inline void add_one_index_to_feature_map(int one_index) {
    for (int a = 0; a < i_num_actions; a++) {
      (*pv_curr_feature_map)[a][(*pv_num_nonzero_in_f)[a]] = 
        one_index +  (*pv_subvector_positions)[a];
      (*pv_num_nonzero_in_f)[a] = (*pv_num_nonzero_in_f)[a] + 1;
    }
            
  }

  /* *********************************************************************
     This method is called at the beginnig of each game
     ******************************************************************** */
  virtual Action on_start_of_game(void);
		
  /* *********************************************************************
     Given the [i,j] indecies of a block, generates a integer vector 
     color_inds, such that color_inds[c] = 1 if the color c exists in 
     the block.
     ******************************************************************** */
  void get_color_ind_from_block(int i, int j, IntVect* color_inds);

  /* *********************************************************************
     Plots the extracted grid. Mostly used for debugging
     ******************************************************************** */
  void plot_extracted_grid();

  FeatureMap* pv_curr_feature_map;
  IntVect* pv_num_nonzero_in_f;
  RLSarsaLambda* p_sarsa_lambda_solver;
  IntArr* pv_tmp_fv_first_part;
  IntVect* pv_tmp_color_bits;
  IntMatrix* pm_background_matrix;
  float f_alpha_multiplier;
  int i_num_block_per_row;
  int i_num_block_per_col;
  int i_num_colors;
  int i_blocks_bits_length;
  int i_base_length;
  int i_full_feature_vec_length;
  IntVect* pv_subvector_positions;
  bool b_end_episode_with_reward;
  static uInt32 pi_eight_bit_pallete[256];
  int i_block_height;
  int i_block_width;
  int i_screen_height;
  int i_screen_width;
  bool b_do_subtract_bg;
  bool b_plot_dirscr_grids;
};

#endif
