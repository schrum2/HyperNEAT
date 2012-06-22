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

#include "search_agent.h"
#include "../emucore/Serializer.hxx"
#include "../emucore/Deserializer.hxx"
#include "../emucore/m6502/src/System.hxx"
#include <sstream>
#include "../common/random_tools.h"
#include "../control/game_controller.h"
#include "tree_node.h"
#include "full_search_tree.h"
#include "uct_search_tree.h"

SearchAgent::SearchAgent(GameSettings* _game_settings, OSystem* _osystem) : 
  PlayerAgent(_game_settings, _osystem),
  p_search_tree(NULL),
  str_curr_state(""),
  i_next_act_frame(0) {
  i_sim_steps_per_node = p_osystem->settings().getInt("sim_steps_per_node", true);
  str_search_method = p_osystem->settings().getString("search_method", true); 
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
  s_cartridge_md5  = p_osystem->console().properties().get(Cartridge_MD5);
  init_simulation_engine();
  e_curr_action = UNDEFINED;
  if (p_game_settings->i_skip_frames_num != 0) {
    p_game_settings->i_skip_frames_num = 0;
    cout << "Forcing i_skip_frames_num to 0, since " 
         << "Skipping frames interferes with simulating the game" << endl;
  }
	
  if (str_search_method == "fulltree") {
    p_search_tree = new FullSearchTree(this);
  } else if (str_search_method == "uct") {
    p_search_tree = new UCTSearchTree(this);
  } else {
    cerr << "Unknown search Method: " << str_search_method << endl;
    exit(-1);
  }
}

SearchAgent::~SearchAgent() {
  delete p_search_tree;
  delete pm_sim_scr_matrix;
  delete pv_sim_ram_content;
}

/* *********************************************************************
   Initilizes the Simulation Engine
   This involves generating a new OSystem and loadign settings for it
   ******************************************************************** */
void SearchAgent::init_simulation_engine() {
  p_sim_event_obj = p_osystem->event();
  p_sim_console = &(p_osystem->console());
  p_sim_system = &(p_sim_console->system());

  // Initilize the screen matrix and RAM vector
  if (p_game_settings->b_uses_screen_matrix) {
    pm_sim_scr_matrix = new IntMatrix;
    assert(i_screen_height > 0);
    assert(i_screen_width > 0);
    for (int i = 0; i < i_screen_height; i++) {
      IntVect row;
      for (int j = 0; j < i_screen_width; j++) {
        row.push_back(-1);
      }
      pm_sim_scr_matrix->push_back(row);
    }
  } else {
    pm_sim_scr_matrix = NULL;
  }
  pv_sim_ram_content = new IntVect(RAM_LENGTH);
}

/* *********************************************************************
   Returns a random action from the set of possible actions
   ******************************************************************** */
Action SearchAgent::agent_step( const IntMatrix* screen_matrix, 
                                const IntVect* console_ram, 
                                int frame_number) {
  Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram,
                                                  frame_number);
  if (special_action != UNDEFINED) {
    return special_action;  // We are resettign or in a delay 
  }
	
  i_curr_num_sim_steps = 0;
  if (i_frame_counter >= i_next_act_frame) {
    // Run a new simulation to find the next action
    i_next_act_frame = i_frame_counter + i_sim_steps_per_node;
    str_curr_state = save_state();
    if (str_search_method == "fulltree") {
      p_search_tree->clear();	// The current full-tree implementation
      // does not support rebuilding the tree
    }
    cout << "Frame: " << i_frame_counter << ", ";
    if (p_search_tree->is_built) {
      // Re-use the old tree
      p_search_tree->move_to_best_sub_branch();
      assert (p_search_tree->get_root_frame_number() == i_frame_counter);
      p_search_tree->update_tree();
      cout << "Tree Updated: ";
    } else {
      // Build a new Search-Tree
      p_search_tree->clear(); 
      p_search_tree->build(str_curr_state, i_frame_counter);
      cout << "Tree Re-Constructed: ";
    }
    e_curr_action = p_search_tree->get_best_action();
    cout << " Root Value = " << p_search_tree->get_root_value();  
    cout << " - Deepest Node Frame: " 
         << p_search_tree->i_deepest_node_frame_num << endl;
    load_state(str_curr_state);
    // deal with the bloody bug, where the screen doesnt get updated
    // after restoring the state for one turn. This *hack* allows 
    // basically skips exporting teh screen for one turn
    i_skip_export_on_frame = i_frame_counter + 1;
  }
  return e_curr_action;
}



/* *********************************************************************
   This method is called when the game ends. 
   ******************************************************************** */
void SearchAgent::on_end_of_game(void) {
  PlayerAgent::on_end_of_game();
  // Our search-tree is useless now. Clear it
  p_search_tree->clear();
}


/* *********************************************************************
   Simulates the game using the given action, for the given number of 
   steps. Returns the total reward recieved, and whether the game 
   eneded while simulating
   If the given action is RADNDOM, we will pick a random action every 
   i_sim_steps_per_node steps  (used in Mante Carlo simulation)
   ******************************************************************** */
void SearchAgent::simulate_game(Action act, int num_steps, int start_frame_num,
                                float& reward, bool& game_ended) {
  reward = 0.0;
  game_ended = false;
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  for (int i = 0; i < num_steps; i++) {
    i_curr_num_sim_steps++;
    if (act == RANDOM && 
        i % i_sim_steps_per_node == 0) {
      act = choice(p_game_settings->pv_possible_actions);
    }
    GameController::apply_action(	p_sim_event_obj, act, PLAYER_B_NOOP);
    p_osystem->myTimingInfo.start = p_osystem->getTicks();
    mediasrc.update(); 
    if (p_game_settings->b_uses_screen_matrix) {
      copy_simulated_framebuffer();
    }
    copy_simulated_ram_content();
    float curr_reward = p_game_settings->get_reward(pm_sim_scr_matrix, 
                                                    pv_sim_ram_content);
    if (curr_reward > 0) {	// convert it to [-1, 0.0, 1.0]  reward
      curr_reward = 1.0;
    } else if (curr_reward < 0) {	
      curr_reward = -1.0;
    } else {
      curr_reward = 0.0;
    }
    reward += curr_reward;
    game_ended = p_game_settings->is_end_of_game(pm_sim_scr_matrix,  
                                                 pv_sim_ram_content, 
                                                 start_frame_num + i);
    if (game_ended) {
      break;
    }
  }
}


/* *********************************************************************
   Saves the OSystem/GameSettings states to a string
   ******************************************************************** */
string SearchAgent::save_state(void) const {
  Serializer ser;
  p_osystem->console().system().saveState(s_cartridge_md5, ser);
  p_game_settings->save_state(ser);
  return ser.get_str();
}

/* *********************************************************************
   Loads the OSystem/GameSettings states from a string
   ******************************************************************** */
void SearchAgent::load_state(const string state_str) {
  Deserializer deser(state_str);
  p_osystem->console().system().loadState(s_cartridge_md5, deser);
  p_game_settings->load_state(deser);
}


/* *********************************************************************
   Copies the content of the simulated framebufer to pm_sim_scr_matrix
   * ****************************************************************** */
void SearchAgent::copy_simulated_framebuffer(void) {
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  uInt8* pi_curr_frame_buffer = mediasrc.currentFrameBuffer();
  // This code section is taken from FrameBufferSoft
  int ind_i, ind_j;
  for (int i = 0; i < i_screen_width * i_screen_height; i++) {
    uInt8 v = pi_curr_frame_buffer[i];
    ind_i = i / i_screen_width;
    ind_j = i - (ind_i * i_screen_width);
    (*pm_sim_scr_matrix)[ind_i][ind_j] = v;
  }
}

/* *********************************************************************
   Copies the content of the simulated RAM to pv_sim_ram_content
   * ****************************************************************** */
void SearchAgent::copy_simulated_ram_content(void) {
  for(int i = 0; i< RAM_LENGTH; i++) {
    (*pv_sim_ram_content)[i] = read_simulated_ram(i);
  }
}


/* ***************************************************************************
 *  Reads a byte from the simulated console ram
 * ***************************************************************************/
int SearchAgent::read_simulated_ram(int offset) {
  offset &= 0x7f; // there are only 128 bytes
  return p_sim_system->peek(offset + 0x80);
}


/* ***************************************************************************
 *  This is a temporary method, used for preparing a demo video.
 *  It should be pretty much ignored!
 * ***************************************************************************/
Action SearchAgent::tmp_prepare_demo_vid(void) {
  if (i_frame_counter < 1250) {
    return PLAYER_A_NOOP;
  }
  if (i_frame_counter < 1300) {
    return PLAYER_A_DOWN;
  }
  str_curr_state = save_state();
  // initilize the screen_matrix
  if (pm_sim_scr_matrix == NULL) {
    pm_sim_scr_matrix = new IntMatrix;
    assert(i_screen_height > 0);
    assert(i_screen_width > 0);
    for (int i = 0; i < i_screen_height; i++) {
      IntVect row;
      for (int j = 0; j < i_screen_width; j++) {
        row.push_back(-1);
      }
      pm_sim_scr_matrix->push_back(row);
    }
  }
  char buffer [50];
  ostringstream filename;
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  for (int a = 0; a < i_num_actions; a++) {
    load_state(str_curr_state);
    GameController::apply_action(p_sim_event_obj, PLAYER_A_NOOP, PLAYER_B_NOOP);
    p_osystem->myTimingInfo.start = p_osystem->getTicks();
    mediasrc.update(); 
    Action curr_act = (*p_game_settings->pv_possible_actions)[a];
    filename.str("");
    filename << "action_" << action_to_string(curr_act)  << "_00.png";
    p_osystem->p_export_screen->save_png(pm_sim_scr_matrix, filename.str());
    for (int i = 0; i < 15; i++) {
      GameController::apply_action(p_sim_event_obj, curr_act, PLAYER_B_NOOP);
      p_osystem->myTimingInfo.start = p_osystem->getTicks();
      mediasrc.update(); 
      copy_simulated_framebuffer();
      sprintf (buffer, "%02d", i + 1);
      filename.str("");
      filename << "action_" << action_to_string(curr_act) << "_" 
               << buffer << ".png";
      p_osystem->p_export_screen->save_png(pm_sim_scr_matrix, filename.str());
    }
  }
  end_game();
  return UNDEFINED;
}
