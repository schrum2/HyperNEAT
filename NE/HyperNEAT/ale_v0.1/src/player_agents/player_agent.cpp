/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  player_agent.cpp
 *
 * The implementation of the PlayerAgent abstract class
 **************************************************************************** */
#include <sstream>
#include "player_agent.h"
#include "random_agent.h"
#include "ram_agent.h"
#include "class_agent.h"
#include "grid_screen_agent.h"
#include "bass_agent.h"
//#include "model_agent.h"
#include "search_agent.h"
#include "single_action_agent.h"
#include "actions_summary_agent.h"
#include "self_detection_agent.h"
#include "freeway_agent.h"
#include "export_tools.h"
#include "game_controller.h"
#include "random_tools.h"
#include "vector_matrix_tools.h"
#include "FSNode.hxx"

/* **********************************************************************
    Constructor
 ********************************************************************* */
PlayerAgent::PlayerAgent(GameSettings* _game_settings, OSystem* _osystem) : 
  p_osystem(_osystem), p_game_settings(_game_settings) {
  pv_possible_actions = p_game_settings->pv_possible_actions;
  pm_curr_screen_matrix = NULL;
  pv_curr_console_ram = NULL;
  i_num_actions = p_game_settings->pv_possible_actions->size();
  e_episode_status = INITIAL_DELAY;
  i_frame_counter = 0;
  i_episode_first_frame = 0;
  i_episode_counter = 0;
  f_episode_reward = 0;
  pv_reward_per_episode = new FloatVect;
  pv_reward_per_frame = new FloatVect;
  pv_episodes_start_frame = new IntVect; 
  pv_episodes_end_frame = new IntVect;
  Settings& settings = p_osystem->settings();
  i_initial_delay_counter = settings.getInt("initial_delay", true);
  i_restart_delay_counter = p_game_settings->i_delay_after_restart;
  i_export_rewards_frq = settings.getInt("export_rewards_frq", true);
  b_export_score_screens = settings.getBool("export_score_screens", true);
  b_export_death_screens = settings.getBool("export_death_screens", true);
  i_max_num_episodes = settings.getInt("max_num_episodes", true);
  i_max_num_frames = settings.getInt("max_num_frames", true);
  i_max_num_frames_per_episode = settings.getInt("max_num_frames_per_episode",
                                                 true);
  i_init_act_explor_count = settings.getInt("init_act_explor_count", true);
  b_minus_one_zero_reward = settings.getBool("minus_one_zero_reward", true);
  b_end_game_with_score = settings.getBool("end_game_with_score", true);
  if (b_minus_one_zero_reward) {
    cout << "Manually inforcing -1/0 reward system" << endl;
  }
  b_do_bg_detection = settings.getBool("do_bg_detection", true);
  if (b_do_bg_detection) {
    cout << "Background-Detection Enabled" << endl;
    p_background_detect = new BackgroundDetector(_game_settings, p_osystem);
  } else {
    p_background_detect = NULL;
  }
  b_do_class_disc = settings.getBool("do_class_disc", true);
  if (b_do_class_disc) {
    cout << "Class-Discovery Enabled" << endl;
    p_class_dicovery = new ClassDiscovery(p_osystem);
  } else {
    p_class_dicovery = NULL;
  }
  b_display_screen = settings.getBool("display_screen", true);
  i_export_screen_frq = settings.getInt("export_frames_frq", true);
  i_export_screen_after = settings.getInt("export_frames_after", true);
  i_export_screen_before = settings.getInt("export_frames_before", true);
  i_export_screens_on_last_n_screens = settings.getInt("export_screens_on_last_n_screens", true);
  i_export_screens_on_last_n_episodes = settings.getInt("export_screens_on_last_n_episodes", true);
  if (i_export_screens_on_last_n_screens > 0) {
    if (i_export_screen_frq <= 0) {
      i_export_screen_frq = 1;
    }
    if (i_max_num_frames <= 0) {
      cerr << "Cannot inforce export_screens_on_last_n_screens, when "
           <<  "max_num_frames is undefined" << endl;
      exit(-1);
    }
    i_export_screen_after = i_max_num_frames - 
      i_export_screens_on_last_n_screens;
    cout << "PlayerAgent: will start Eporting screens after frame #" 
         << i_export_screen_after << endl;
  }
  i_skip_export_on_frame	= -1;
  if (i_export_screen_frq > 0 && 
      (!FilesystemNode::dirExists("exported_screens"))) {
    // Make a subdirectory to save the screen-shots
    FilesystemNode::makeDir("exported_screens");
  }
  if (i_export_screens_on_last_n_episodes > 0) {
    if (i_max_num_episodes <= 0) {
      cerr << "Cannot inforce i_export_screens_on_last_n_episodes, when "
           <<  "i_max_num_episodes is undefined" << endl;
      exit(-1);
    }
    i_export_screen_frq = 0;	// we will set this later
  }

  i_curr_expl_act_index = 0;
  i_curr_act_frame_count = i_init_act_explor_count;
  b_reward_on_this_frame = false;
}

/* **********************************************************************
   Deconstructor
   ********************************************************************* */
PlayerAgent::~PlayerAgent() {
  delete pv_reward_per_episode;
  delete pv_reward_per_frame;
  delete pv_episodes_start_frame; 
  delete pv_episodes_end_frame;
  if (p_background_detect) {
    delete p_background_detect;
  }
  if (p_class_dicovery) {
    delete p_class_dicovery;
  }
}

/* *********************************************************************
   The main method. given a 2D array of the color indecies on the
   screen, and the content of the consoel RAM  this method 
   will decides the next action based on the desired algorithm.
   The implementation in the superclass takes care of restarting the 
   game at the end, pressing the first action (if defined), and 
   countin frames and episodes. It should be called from all 
   overriden functions
   ******************************************************************** */
Action PlayerAgent::agent_step(const IntMatrix* screen_matrix, 
                               const IntVect* console_ram, 
                               int frame_number) {                                
  i_frame_counter = frame_number;

  if (i_max_num_frames != -1 && 
      i_frame_counter > i_max_num_frames) {
    cout << "Max number of frames: " << i_max_num_frames 
         << " reached" << endl;
    end_game();
  }
		
  pm_curr_screen_matrix = screen_matrix; 
  pv_curr_console_ram = console_ram;     
	
  // Export the Screen
  if ( i_export_screen_frq != 0 && 
       (i_frame_counter % i_export_screen_frq == 0) && 
       (i_frame_counter >= i_export_screen_after) && 
       (i_export_screen_before == -1 || 
        i_frame_counter <= i_export_screen_before)) {
    export_screen(pm_curr_screen_matrix);
  }

  f_curr_reward = p_game_settings->get_reward(screen_matrix, console_ram);
  pv_reward_per_frame->push_back(f_curr_reward);
  if (f_curr_reward > 0) { 
    b_reward_on_this_frame = true;
  } else {
    b_reward_on_this_frame = false;
  }
  if (b_minus_one_zero_reward) {
    // Use the -1/0 reward system, i.e. -1 reward on all state, 0 on goal 
    if (f_curr_reward > 0) {
      f_curr_reward = 0.0;
    } else {
      f_curr_reward = -1.0;
    }
  }

  if (e_episode_status == AGENT_ACTING || 
      e_episode_status == ACTION_EXPLOR) {
    f_episode_reward += f_curr_reward;
  }
	
  if ((b_export_score_screens == true) && b_reward_on_this_frame) {
    ostringstream filename;
    filename << "reward_screen__frame_" << i_frame_counter << 
      "__reward_" << (int) f_curr_reward << ".png";
    p_osystem->p_export_screen->save_png(screen_matrix, filename.str());
  }

  switch (e_episode_status) {
  case INITIAL_DELAY:
    // We are at the initial delay stage (i.e. do nothing for n frames,
    // before even resetting the game)
    if (i_initial_delay_counter > 0) {
      i_initial_delay_counter--;
      return choice <Action> (p_game_settings->pv_possible_actions);;
    } else {
      // switch to the next state: restarting the game and 
      // going through the restart delay
      e_episode_status = RESTART_DELAY;
      i_restart_delay_counter = p_game_settings->i_delay_after_restart;
      return RESET;
    }
    break;
  case RESTART_DELAY:
    // We are at the restart delay stage (i.e. doing nothing for a 
    // number frames, after resettng the game)
    if (i_restart_delay_counter > 0) {
      i_restart_delay_counter--;
      return PLAYER_A_NOOP;
    } else {
      // In the next state, we hand the game to the plaeyr agent 
      // and either return the FIRST_ACTION, or whatever
      // the on_start_of_game() returns
      i_episode_first_frame = i_frame_counter;
      if (p_game_settings->e_first_action != UNDEFINED) {
        e_episode_status = START_EPISODE;
        cout << "performed first_action on fram #" << i_frame_counter << endl;
        return p_game_settings->e_first_action;
      } else {
        if (i_init_act_explor_count > 0 && 
            i_curr_expl_act_index < i_num_actions) {
          e_episode_status = ACTION_EXPLOR;
          cout << "Exploring action: " << i_curr_expl_act_index <<
            "(" << 
            action_to_string((*pv_possible_actions)[i_curr_expl_act_index]) 
               << ") for " << i_curr_act_frame_count << 
            " more frames." << endl;
        } else {
          e_episode_status = AGENT_ACTING;
        }
        return on_start_of_game();
      }
    }
    break;
  case START_EPISODE: {
    // We Are about to start a new episode
    if (i_init_act_explor_count > 0 && 
        i_curr_expl_act_index < i_num_actions) {
      e_episode_status = ACTION_EXPLOR;
      cout << "Exploring action: " << i_curr_expl_act_index <<
        "(" << 
        action_to_string((*pv_possible_actions)[i_curr_expl_act_index]) 
           << ") for " << i_curr_act_frame_count << 
        " more frames." << endl;
    } else {
      e_episode_status = AGENT_ACTING;
    }
    return on_start_of_game();
    break;
  }
  case ACTION_EXPLOR:
    // We are trying each action for a number of frames
    if (i_curr_act_frame_count == 0) {
      // Goto the next action
      i_curr_expl_act_index++;
      i_curr_act_frame_count = i_init_act_explor_count;
      if (i_curr_expl_act_index == i_num_actions) {
        // We have tried all the actions
        e_episode_status = AGENT_ACTING;
      } else {
        cout << "Exploring action: " << i_curr_expl_act_index 
             << "(" 
             << action_to_string((*pv_possible_actions)[i_curr_expl_act_index]) 
             << ") for " << i_curr_act_frame_count << " frames." << endl;
      }
    } else {
      i_curr_act_frame_count--;
    }
    // NOTE: no break here! we still have to check for the end of the 
    // game.
			
  case AGENT_ACTING: {
    // We are acting in the word.
    // See if the game has eneded, and if so, restart the game
    bool game_ended = p_game_settings->is_end_of_game(screen_matrix,  
                                                      console_ram, i_frame_counter);
    if (b_end_game_with_score && b_reward_on_this_frame) {
      game_ended = true;
    }
    int num_frames_in_episode = i_frame_counter - i_episode_first_frame;
    if (i_max_num_frames_per_episode != -1 && 
        num_frames_in_episode > i_max_num_frames_per_episode) {
      cout << "Reached maximum frames per episode. "<< endl;
      p_game_settings->reset_state();
      game_ended = true;
    }
    if (game_ended) {
      // take care of end-of-game business
      on_end_of_game();
      // switch to the next state: restarting the game and 
      // going through the restart delay
      e_episode_status = RESTART_DELAY;
      i_restart_delay_counter=p_game_settings->i_delay_after_restart;
      return RESET;
    } else {
      if (b_do_bg_detection) {
        // Send screen for background detection
        p_background_detect->get_new_screen(pm_curr_screen_matrix);
        if (p_background_detect->is_bg_extraction_complete()) {
          cout << "Background Detection Complete." << endl;
          end_game();
        }
      }
      if (b_do_class_disc) {
        // Send screen fro background detection
        p_class_dicovery->get_new_screen(pm_curr_screen_matrix, 
                                         i_frame_counter);
        if (p_class_dicovery->is_class_discovery_complete()) {
          cout << "Class Discovery Complete." << endl;
          end_game();
        }
      }
      return UNDEFINED; // let the subclass decide what to do
    }
    break;
  }
  default:
    assert(false); // we should never reach here
  }
  assert(false); // we should never reach here
  return UNDEFINED;
}
                              

/* *********************************************************************
   This method is called when the game ends. The superclass 
   implementation takes care of counting number of episodes, and 
   saving the reward history. Any other update should be taken care of
   in the derived classes
   ******************************************************************** */
void PlayerAgent::on_end_of_game(void) {
  pv_reward_per_episode->push_back(f_episode_reward);
  int num_frames = i_frame_counter - i_episode_first_frame;
  pv_episodes_start_frame->push_back(i_episode_first_frame);
  pv_episodes_end_frame->push_back(i_frame_counter);
	
  cout << "Episode #" << i_episode_counter <<
    ", Frame #" << i_frame_counter << 
    ", Num Frames = " << num_frames <<
    ", Sum Reward = " << f_episode_reward << endl;
  f_episode_reward = 0;
  i_episode_counter++;
    
  if ((i_export_rewards_frq != 0) &&
      (i_episode_counter % i_export_rewards_frq == 0)) {
    // Export the rewards-history
    ostringstream filename;
    filename << "reward_per_episode__epis_0_" << i_episode_counter << ".txt";
    export_vector(pv_reward_per_episode, filename.str());
    filename.str("");
    filename << "reward_per_frame__frame_0_" << i_frame_counter << ".txt";
    export_vector(pv_reward_per_frame, filename.str());
    filename.str("");
    filename << "epis_start_frame__epis_0_" << i_episode_counter << ".txt";
    export_vector(pv_episodes_start_frame, filename.str());
    filename.str("");
    filename << "epis_end_frame__epis_0_" << i_episode_counter << ".txt";
    export_vector(pv_episodes_end_frame, filename.str());
  }
  if (b_export_death_screens == true) {
    ostringstream filename;
    filename << "death_screen__frame_" << i_frame_counter << ".png";
    p_osystem->p_export_screen->save_png(pm_curr_screen_matrix, 
                                         filename.str());
  }
  if (i_export_screens_on_last_n_episodes > 0 &&
      ( i_max_num_episodes - i_episode_counter <= 
        i_export_screens_on_last_n_episodes)) {
    i_export_screen_frq = 1;	// start exporting screens
  }
  if ((i_max_num_episodes != -1) && 
      (i_episode_counter > i_max_num_episodes)) {
    cout << "Max number of episodes: " << i_max_num_episodes 
         << " reached." << endl;
    end_game();
  }
}

/* *********************************************************************
   Generates an instance of one of the PlayerAgent subclasses, based on
   "player_agent" value in the settings.  
   Returns a NULL pointer if the value of player_agent is invalid.
   Note 1: If you add a new player_agent subclass, you need to also 
   add it here
   Note 2: The caller is resposible for deleting the returned pointer
   ******************************************************************** */
PlayerAgent* PlayerAgent::generate_agent_instance(
                                                  GameSettings* _game_settings,
                                                  OSystem* _osystem) {
  string player_agent = _osystem->settings().getString("player_agent");
  PlayerAgent* new_agent = NULL;
  if (player_agent == "random_agent") {
    cout << "Game will be controlled by Random Agent" << endl;
    new_agent = new RandomAgent(_game_settings, _osystem);
  } else if (player_agent == "ram_agent") {
    cout << "Game will be controlled by RAM Agent" << endl;
    new_agent = new RAMAgent(_game_settings, _osystem);
  } else if (player_agent == "class_agent") {
    cout << "Game will be controlled by Class Agent" << endl;
    new_agent = new ClassAgent(_game_settings, _osystem);
  } else if (player_agent == "gridscr_agent") {
    cout << "Game will be controlled by Direct-Screen Agent" << endl;
    new_agent = new GridScrAgent(_game_settings, _osystem);
  } else if (player_agent == "bass_agent") {
    cout << "Game will be controlled by bass agent" << endl;
    new_agent = new BassAgent(_game_settings, _osystem);
  // } else if (player_agent == "model_agent") {
  //   cout << "Game will be controlled by model agent" << endl;
  //   new_agent = new ModelAgent(_game_settings, _osystem);
  } else if (player_agent == "search_agent") {
    cout << "Game will be controlled by Search-Agent" << endl;
    new_agent = new SearchAgent(_game_settings, _osystem);
  } else if (player_agent == "single_action") {
    cout << "Game will be controlled by Single-Action Agent, with " 
         << "action-index = " 
         << _osystem->settings().getString("single_action_index") << endl;
    new_agent = new SingleActionAgent(_game_settings, _osystem);
  } else if (player_agent == "actions_summary") {
    cout << "Game will be controlled by Action-Summary Agent" << endl;
    new_agent = new ActionSummaryAgent(_game_settings, _osystem);
  }else if (player_agent == "freeway_agent") {
    cout << "Game will be controlled by Freeway-Agent" << endl;
    new_agent = new FreewayAgent(_game_settings, _osystem);
  } else if (player_agent == "self_detection_agent") {
    cout << "Game will be controlled by Self detection agent" << endl;
    new_agent = new SelfDetectionAgent(_game_settings, _osystem);
  } else {
    cerr << "PlayerAgent::generate_agent_instance: I don't know" <<
      " what PlayerAgent to use for player_agent value:" << 
      player_agent << endl;
    exit(-1);
  }
  return new_agent;
}

/* *********************************************************************
   Finishes the game and saves anythign that might need saving
   ******************************************************************** */
void PlayerAgent::end_game(void) {
  cout << "quitting..." << endl;
  ostringstream filename;
  filename << "reward_per_episode__epis_0_" << i_episode_counter << ".txt";
  export_vector(pv_reward_per_episode, filename.str());
  filename.str("");
  filename << "reward_per_frame__frame_0_" << i_frame_counter << ".txt";
  export_vector(pv_reward_per_frame, filename.str());
  filename.str("");
  filename << "epis_start_frame__epis_0_" << i_episode_counter << ".txt";
  export_vector(pv_episodes_start_frame, filename.str());
  filename.str("");
  filename << "epis_end_frame__epis_0_" << i_episode_counter << ".txt";
  export_vector(pv_episodes_end_frame, filename.str());	
	
  int avg_frames_num = i_max_num_frames_per_episode * 1000;
  if (i_frame_counter > avg_frames_num) {
    float last_steps_avg = get_vector_average(pv_reward_per_frame, 
                                              i_frame_counter - avg_frames_num,
                                              i_frame_counter);
    cout << endl << "Average reward for the last " << avg_frames_num 
         << " frames is " << last_steps_avg << endl;
    // Write a summary file
    ofstream file;
    file.open("run_summary.txt");
    file << "Player Agent: " 
         << p_osystem->settings().getString("player_agent", true) << endl
         << "Game: " << p_game_settings->s_rom_file << endl
         << "Number of Frames: " << i_frame_counter << endl
         << "Number of Episodes: " << i_episode_counter << endl
         << "Average Reward on last " << avg_frames_num << " frames: "
         << last_steps_avg << endl;
    file.close();
  } else {
    cout << "Did not recieve enough frames to generate average reward for " 
         << avg_frames_num << " frames." << endl;
  }
  p_osystem->quit(); // goodbye cruel world!	
}


/* *********************************************************************
    Takes care of exporting the screen (if neccessary)
 * ****************************************************************** */
void PlayerAgent::export_screen(const IntMatrix * screen_matrix) {
  if (i_frame_counter == i_skip_export_on_frame) {
    return;
  }

  // Export the current screen to a PNG file
  ostringstream filename;
  char buffer [50];
  sprintf (buffer, "%09lld", i_frame_counter);
  filename << "exported_screens/frame_" << buffer << ".png";
  p_osystem->p_export_screen->save_png(screen_matrix, filename.str());
}

/* *********************************************************************
   Displays the current screen from a png file of it.
 * ****************************************************************** */
void PlayerAgent::display_screen(const IntMatrix& screen_matrix) {
  if (!b_display_screen)
    return;
  
  p_osystem->p_display_screen->display_screen(screen_matrix, screen_matrix[0].size(), screen_matrix.size());
}
