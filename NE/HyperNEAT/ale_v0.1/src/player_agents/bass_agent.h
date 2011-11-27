/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  bass_agent.h
 *
 *  The bass agent is like the grid screen agent but defines cleaner interfaces.
 **************************************************************************** */


#ifndef BASS_AGENT_H
#define BASS_AGENT_H

#include "common_constants.h"
#include "player_agent.h"

class BassAgent : public PlayerAgent {
 public:
  BassAgent(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~BassAgent();

  Action agent_step(const IntMatrix* screen_matrix, 
                    const IntVect* console_ram, 
                    int frame_number);

  /**********************************************************************
   * This method passes in the bass feature vector. It should be ovridden
   * by a subclass.
   *********************************************************************/
  virtual Action agent_step(IntArr* pv_feature_vec);

  virtual void on_end_of_game(void);

  /**********************************************************************
   * Choose what should be displayed...
   *********************************************************************/
  virtual void displayScreen();


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
  void generate_feature_vec(void);
  void get_color_ind_from_block(int i, int j, IntVect* color_bits);
  void plot_extracted_grid(IntArr* pv_feature_vec);

  // Functions for displaying bass content
  void display_bass_screen();

  // Screen info
  int i_screen_height;
  int i_screen_width;
  int i_num_colors;
  
  bool b_do_subtract_bg;

  // Block info
  int i_num_block_per_row;
  int i_num_block_per_col;
  int i_block_height;
  int i_block_width;
  int i_blocks_bits_length;

  IntVect*   pv_tmp_color_bits;
  IntArr*    pv_feature_vec;
  IntMatrix* pm_background_matrix;

  static uInt32 pi_eight_bit_pallete[256];
};


#endif
