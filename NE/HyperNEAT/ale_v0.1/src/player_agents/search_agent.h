
/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  search_agent.h
 *
 * The implementation of the SearchAgent class, which uses Search Algorithms
 * to act in the game
 **************************************************************************** */

#ifndef SEARCH_AGENT_H
#define SEARCH_AGENT_H

#include "common_constants.h"
#include "player_agent.h"
#include "OSystem.hxx"
#include "SettingsUNIX.hxx"
#include "OSystemUNIX.hxx"
#include "search_tree.h"

class SearchAgent : public PlayerAgent {
  friend class SearchTree;
  friend class FullSearchTree;
  friend class UCTSearchTree;
  friend class TreeNode;
 public:
  SearchAgent(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~SearchAgent();
		
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

  /* *********************************************************************
     Returns the number of simulate steps during the current turn
     ******************************************************************** */
  int get_num_simulated_steps(void) const {return i_curr_num_sim_steps;}


 protected:

  /* *********************************************************************
     Initilizes the Simulation Engine
     This involves generating a new OSystem and loadign settings for it
     ******************************************************************** */
  void init_simulation_engine();
		
  /* *********************************************************************
     Simulates the game using the given action, for the given number of 
     steps. Returns the total reward recieved, and whether the game 
     eneded while simulating
     ******************************************************************** */
  void simulate_game(Action act, int num_steps, int start_frame_num,
                     float& reward, bool& game_ended);
		
  /* *********************************************************************
     Saves the OSystem's state to string
     ******************************************************************** */
  string save_state(void) const;
		
  /* *********************************************************************
     Saves the OSystem's state to string
     ******************************************************************** */
  void load_state(const string state_str);

  /* *********************************************************************
     Copies the content of the simulated framebufer to pm_sim_scr_matrix
     * ****************************************************************** */
  void copy_simulated_framebuffer(void);
		
  /* *********************************************************************
     Copies the content of the simulated RAM to pv_sim_ram_content
     * ****************************************************************** */
  void copy_simulated_ram_content(void);

  /* ***************************************************************************
   *  Reads a byte from the simulated console ram
   * ***************************************************************************/
  int read_simulated_ram(int offset);
		
  /* ***************************************************************************
   *  This is a temporary method, used for preparing a demo video.
   *  It should be pretty much ignored!
   * ***************************************************************************/
  Action tmp_prepare_demo_vid(void);
		
  string str_search_method;	// [uct/fulltree]
  Action e_curr_action;	        // The action we are curently taking
  SearchTree* p_search_tree;	// search-tree object used to search down the game tree
  Console* p_sim_console;	// Ponter to the simulation console
  System* p_sim_system;	        // Pointer to the emulator system
  SettingsUNIX* p_sim_settings; // Pointer to settings used for simulation
  Event* p_sim_event_obj;       // Pointer to the simulated game's event object
  IntMatrix* pm_sim_scr_matrix; // 2D Matrix containing screen pixel colors
  IntVect* pv_sim_ram_content;  // Content of the emulator RAM
  string s_cartridge_md5;	// Cartridge MD5
  string str_curr_state;	// Saves the current state of the game
  int i_sim_steps_per_node;	// Number of steps we will run the simulation in each search-tree node
  int i_next_act_frame;	        // The next frame where we need to pick an action
  int i_curr_num_sim_steps;     // Number of simulate dsteps during the current turn
  int i_screen_height;
  int i_screen_width;
};

#endif




