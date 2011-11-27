/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  player_agent.h
 *  stella_agent
 *
 * The implementation of the PlayerAgent abstract class
 **************************************************************************** */
#ifndef PLAYER_AGENT_H
#define PLAYER_AGENT_H

#include <stdio.h>
#include "OSystem.hxx"
#include "common_constants.h"
#include "game_settings.h"
#include "background_detector.h"
#include "class_discovery.h"

class PlayerAgent  {
  /* *************************************************************************
     This is an abstract class and should be the Superclass for all
     agent classes that want to interact with stella
        
     Instance variabls:
     - p_game_settings           An instance of the GameSettings class
     - pm_curr_screen_matrix     2D matrix of color indecies
     - pv_curr_console_ram       Content of the Console RAM
     - i_num_actions             Number of possible acitons
     - p_osystem                 Pointer to the stella's OSystem object
     - p_background_detect		Used for background-detection
     - p_class_dicovery			Used for class-discovery
     - e_episode_counter         The status of the current episode.Can be
     INITIAL_DELAY,      The delay at very beginning of game
     RESTART_DELAY,      Delay after restarting the game
     AGENT_ACTING,       we are in the middle of an episode       
     - i_frame_counter           Counts the number of frames we have seen
     - i_episode_counter         Counts the number of episodes we've seen
     - f_episode_reward          Reward we have accumilated in current 
     episode
     - pv_reward_per_episode     rewards recieved per episode
     - pv_reward_per_frame		rewards recieved per frame
     - i_initial_delay_counter   At the very begining, we wait this many
     frames (Some games need a little time,
     before we can push restart)
     - i_restart_delay_counter   After restarting the game, we wait
     this many frames. (Some games won't 
     allow you to do anything for a few 
     frames after restarting).
     - pv_possible_actions       List of allowed actions in the game
     - i_init_act_explor_count   At the very begining, we will try
     repeating each action this many times 
     (this is a mthod for exploration)
     - i_curr_expl_act_index		The index of the action we are currently 
     exploring
     - i_curr_act_frame_count	How many frames we have tried this 
     action
     - b_minus_one_zero_reward	When true, we will use the -1/0 reward 
     system
     - b_do_bg_detection			When true, the player-agent will also
     do backgroudn detection 
     - b_do_class_disc			When true, we will do class-discovery
     ************************************************************************* */
 public:
  PlayerAgent(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~PlayerAgent();
        
  /* *********************************************************************
     The main method. given a 2D array of the color indecies on the
     screen, and the content of the consoel RAM  this method 
     will decides the next action based on the desired algorithm.
     The implementation in the superclass takes care of restarting the 
     game at the end, pressing the first action (if defined), and 
     countin frames and episodes. It should be called from all 
     overriden functions
     ******************************************************************** */
  virtual Action agent_step(  const IntMatrix* screen_matrix, 
                              const IntVect* console_ram, 
                              int frame_number);
        
  /* *********************************************************************
     This method is called when the game ends. The superclass 
     implementation takes care of counting number of episodes, and 
     saving the reward history. Any other update should be taken care of
     in the derived classes
     ******************************************************************** */
  virtual void on_end_of_game(void);

  /* *********************************************************************
     This method is called at the beginnig of each game
     ******************************************************************** */
  virtual Action on_start_of_game(void) {
    return UNDEFINED;
  };

  /* *********************************************************************
     Takes care of exporting the screen (if neccessary)
     * ****************************************************************** */
  void export_screen(const IntMatrix * screen_matrix);

  /* *********************************************************************
     Displays the screen by loading the exported PNG file, deleting if
     necessary.
     * ****************************************************************** */
  virtual void display_screen(const IntMatrix& screen_matrix);
        
  /* *********************************************************************
     Generates an instance of one of the PlayerAgent subclasses, based on
     "player_agent" value in the settings. 
     Note 1: If you add a new player_agent subclass, you need to also 
     add it here
     Note 2: The caller is resposible for deleting the returned pointer
     ******************************************************************** */
  static PlayerAgent* generate_agent_instance(GameSettings* _game_settings,
                                              OSystem* _osystem);
        
  int i_episode_counter;			  // number of episodes we have seen
  long long i_frame_counter;        // number of frames we have seen
  long long i_episode_first_frame;  // The frame where the episode starts
  float f_episode_reward;           // Reward in the current episode
  FloatVect* pv_reward_per_episode; // Rewards recieved per episode
  FloatVect* pv_reward_per_frame;	  // Rewards recieved per frame
  IntVect* pv_episodes_start_frame; // frame number at beginning each epis
  IntVect* pv_episodes_end_frame;   // frame number at end of each episode

 protected:
  /* *********************************************************************
     Finishes the game and saves anythign that might need saving
     ******************************************************************** */
  void end_game(void);
		
  OSystem* p_osystem;               // Pointer to the stella's OSystem 
  GameSettings* p_game_settings;  // An instance of the GameSettings class
  const IntMatrix* pm_curr_screen_matrix; // 2D matrix of color indecies
  const IntVect* pv_curr_console_ram;     // Content of the Console RAM
  BackgroundDetector* p_background_detect;// Used for background-detection
  ClassDiscovery* p_class_dicovery; // Used for class-discovery
        
		
  float f_curr_reward;              // Reward recieved from game
  int i_num_actions;                // Number of possible acitons
  EpisodeStatus e_episode_status;   // The status of the current episode
  int i_initial_delay_counter;      // Initial delay counter
  int i_restart_delay_counter;      // Restart delay counter
  const ActionVect* pv_possible_actions;  
  int i_export_rewards_frq;         // How often we should export the 
  // rewards history
  int i_max_num_episodes;           // We exit the program after this 
  // number of episodes
  int i_max_num_frames;			  // We exit the program after this 
                                          // number of frames
  int i_max_num_frames_per_episode; // Episode ends after this many frames		
  bool b_export_score_screens;      // When true, we will save the screens
  // with non-zero rewards
  bool b_export_death_screens;      // When true, we will save the screens
  // when we die
  int i_init_act_explor_count;
  int i_curr_expl_act_index;		  // The index of the action we 
  // are currently exploring
  int i_curr_act_frame_count;		  // How many frames we have tried this 
  // action
  bool b_minus_one_zero_reward;	  // When true, we will use the -1/0  
  // reward system
  bool b_reward_on_this_frame;	  // True if we have got a reward on
  // this frame.
  bool b_end_game_with_score;		  // When true, the game ends after 
  // each score recieved. 
  bool b_do_bg_detection;			  // When true, the player-agent will
  // also do backgroudn detection 
  bool b_do_class_disc;			  // When true, will do class-discovery
  bool b_display_screen;                    // When true we will display screen during game
  int i_export_screen_frq;		  // How often we'll export the 
  // screen 0 means never
  int i_export_screen_after;		  // Exporting frames starts after  
  // this many frams
  int i_export_screen_before;		  // Exporting the frames stops
  // after this many frames
  int i_export_screens_on_last_n_screens; // *Only* export screens on the 
  // last n screens
  int i_export_screens_on_last_n_episodes; // *Only* export screens on the 
  // last n screens
  int i_skip_export_on_frame;		  // We will skip this frame number
  // from exporting. This is a *hack*
  // to get around the emulation
  // jump-screen bug.
};



#endif
