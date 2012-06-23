/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  internal_controller.cpp
 *
 *  The implementation of the InternalController class, which is a subclass of 
 * GameConroller, and is resposible for sending the Screens/RAM content to 
 * whatever player_agent we are using to control the game
 **************************************************************************** */

#include <sstream>
#include "internal_controller.h"
#include "game_controller.h"
#include "../player_agents/game_settings.h"
#include "../player_agents/player_agent.h"
#include "../common/random_tools.h"
#include "../emucore/m6502/src/System.hxx"


/* *********************************************************************
   Constructor
   ******************************************************************** */
InternalController::InternalController(OSystem* _osystem)  : 
  GameController(_osystem) {
  string rom_file = p_osystem->settings().getString("rom_file");
  p_game_settings = GameSettings::generate_game_Settings_instance(rom_file);
  i_skip_frames_num = p_game_settings->i_skip_frames_num;
  pm_screen_matrix = NULL;
  if (b_send_screen_matrix) {
    pm_screen_matrix = new IntMatrix;
    assert(i_screen_height > 0);
    assert(i_screen_width > 0);
    for (int i = 0; i < i_screen_height; i++) {
      IntVect row;
      for (int j = 0; j < i_screen_width; j++) {
        row.push_back(-1);
      }
      pm_screen_matrix->push_back(row);
    }
  } 
  pv_ram_content = NULL;
  if (b_send_console_ram) {
    pv_ram_content = new IntVect(RAM_LENGTH);
  } 
    
  p_player_agent_right = NULL; // Change this if you want a right player
  // Note that current agents only produce action 
  // for the left player, and need to be fixed
  p_player_agent_left = PlayerAgent::generate_agent_instance(p_game_settings, 
                                                             p_osystem);
  i_frame_number = 0;
}
    
/* *********************************************************************
   Deconstructor
   ******************************************************************** */
InternalController::~InternalController() {
  if (p_game_settings) {
    delete p_game_settings;
  }
  if (p_player_agent_right) {
    delete p_player_agent_right;
  }
  if (p_player_agent_left) {
    delete p_player_agent_left;
  }
  if (pv_ram_content) {
    delete pv_ram_content;
  }
  if (pm_screen_matrix) {
    delete pm_screen_matrix;
  }
}

        
/* *********************************************************************
   This is called on every iteration of the main loop. It is resposible 
   passing the framebuffer and the RAM content to whatever AI module we 
   are using, and applying the returned actions.
   * ****************************************************************** */
void InternalController::update() {
  Action player_a_action, player_b_action;
  // See if we are skipping this frame
  if (i_skip_frames_counter < p_game_settings->i_skip_frames_num) {
    // skip this frame
    i_skip_frames_counter++;
    if (e_previous_a_action == RESET || e_previous_b_action == RESET ) {
      cout << "reset repeated!" <<endl;
      player_a_action = PLAYER_A_NOOP;
      player_b_action = PLAYER_B_NOOP;
      e_previous_a_action = PLAYER_A_NOOP;
      e_previous_b_action = PLAYER_B_NOOP;
    } else {
      player_a_action = e_previous_a_action;
      player_b_action = e_previous_b_action;
    }
  } else {
    // don't skip this frame
    i_skip_frames_counter = 0;
    i_frame_number++;
    // Update the screen and ram vectors
    copy_framebuffer();
    copy_ram_content();
    // Send screen matrix / ram to PLayerAgent and apply the returned action
    if (p_player_agent_left) {
      player_a_action = p_player_agent_left->agent_step(pm_screen_matrix, 
                                                        pv_ram_content,
                                                        i_frame_number);
      p_player_agent_left->display_screen(*pm_screen_matrix);
    } else {
      player_a_action = PLAYER_A_NOOP;
    }
    if (p_player_agent_right) {
      player_b_action = p_player_agent_right->agent_step(pm_screen_matrix, 
                                                         pv_ram_content,
                                                         i_frame_number);
      p_player_agent_right->display_screen(*pm_screen_matrix);
    } else {
      player_b_action = PLAYER_B_NOOP;
    }
  }
  e_previous_a_action = player_a_action;
  e_previous_b_action = player_b_action;
  apply_action(p_global_event_obj, player_a_action, player_b_action);
}



/* *********************************************************************
   Copies the content of the framebufer to pm_screen_matrix
   * ****************************************************************** */
void InternalController::copy_framebuffer(void) {
  if (!b_send_screen_matrix) {
    return;
  }
  // This code section is taken from FrameBufferSoft
  int ind_i, ind_j;
  for (int i = 0; i < i_screen_width * i_screen_height; i++) {
    uInt8 v = pi_curr_frame_buffer[i];
    ind_i = i / i_screen_width;
    ind_j = i - (ind_i * i_screen_width);
    (*pm_screen_matrix)[ind_i][ind_j] = v;
  }
}

/* *********************************************************************
   Copies the content of RAM to pv_ram_content
   * ****************************************************************** */
void InternalController::copy_ram_content(void) {
  if (!b_send_console_ram) {
    return;
  }
  for(int i = 0; i< RAM_LENGTH; i++) {
    (*pv_ram_content)[i] = read_ram(i);
  }
}

