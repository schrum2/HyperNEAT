/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  actions_summary_agent.cpp
 *
 * The implementation of the ActionSummaryAgent class, which generates a summary
 * of taking each action in the game for num_episodes_to_avg_results episodes
 **************************************************************************** */

#include "actions_summary_agent.h"
#include "../common/random_tools.h"
#include "../common/vector_matrix_tools.h"
#include "../common/export_tools.h"
#include <fstream>

ActionSummaryAgent::ActionSummaryAgent(	GameSettings* _game_settings, 
										OSystem* _osystem) : 
    PlayerAgent(_game_settings, _osystem){
	i_curr_action_index = -1;
	str_current_action = "RANDOM";
	i_act_episode_counter = 0;	
	i_max_episode_per_act = p_osystem->settings().getInt(
										"num_episodes_to_avg_results", true);
	for (int i = 0; i < LAST_ACTION_INDEX; i++) {
		v_results_sum.push_back(0.0);
	}
	int min_epis_required =  i_max_episode_per_act * (i_num_actions + 2);
	if (i_max_num_episodes != -1 && 
		i_max_num_episodes < min_epis_required) {
		cout << "MaxNumEpisodes updated form: " << i_max_num_episodes
			 << " to: " << min_epis_required << endl;
		i_max_num_episodes = min_epis_required;
	}
	int min_frames_req = (min_epis_required + 1) * 
						 (i_max_num_frames_per_episode + 
						  p_game_settings->i_delay_after_restart + 100);
	if (i_max_num_frames != -1 &&
		i_max_num_frames < min_frames_req) {
		cout << "MaxNumFrames updated form: " << i_max_num_frames
			 << " to: " << min_frames_req << endl;
		i_max_num_frames = min_frames_req;
	}
}

/* *********************************************************************
	Returns a random action from the set of possible actions
 ******************************************************************** */
Action ActionSummaryAgent::agent_step(  const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_number) {
	Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram,
													frame_number);
    if (special_action != UNDEFINED) {
        return special_action;  // We are resettign or in a delay 
    }

	if (i_curr_action_index == -1) {
		// return a random action
		return choice <Action> (p_game_settings->pv_possible_actions);
	}
	return (*p_game_settings->pv_possible_actions)[i_curr_action_index];
}

/* *********************************************************************
	This method is called when the game ends. The superclass 
	implementation takes care of counting number of episodes, and 
	saving the reward history. Here we check to see if we need to 
	swtch to the next action, and also update the episode counter and
	v_act_epis_rewards
******************************************************************** */
void ActionSummaryAgent::on_end_of_game(void) {
	v_act_epis_rewards.push_back(f_episode_reward);
	i_act_episode_counter++;
	if (i_act_episode_counter > i_max_episode_per_act) {
		// Save the average reward for this action
		float average_reward = get_vector_average(	&v_act_epis_rewards, 0, 
													v_act_epis_rewards.size());
		int ind;
		if (i_curr_action_index == -1) {
			ind = RANDOM;
		} else {
			ind = (*pv_possible_actions)[i_curr_action_index];
		}
		v_results_sum[ind] = average_reward;
		// Export the rewards 
		string filename = "reward_per_episode__act_" + str_current_action + ".txt";
		export_vector(&v_act_epis_rewards, filename);
		// switch to the next action
		i_curr_action_index++;
 		if (i_curr_action_index == i_num_actions) {
			// we are done
			save_results_summary();
			end_game();
		}
		str_current_action = action_to_string(
								(*pv_possible_actions)[i_curr_action_index]);
		cout << " ** Switching to action:" << str_current_action << " **" << endl;
		i_act_episode_counter = 0;
		v_act_epis_rewards.clear();
	}
	PlayerAgent::on_end_of_game();
}

/* *********************************************************************
	Saves the summary results to a text file
******************************************************************** */	
void ActionSummaryAgent::save_results_summary(void) {
	ofstream file;
	file.open("actions_summary.txt");
	file << "action,average_reward_over_" 
		 << i_max_episode_per_act << "_episodes" << endl;
	file << "RANDOM," << v_results_sum[RANDOM] << endl;
	for (unsigned int a = 0; a < i_num_actions; a++) {
		file << action_to_string((*pv_possible_actions)[a]) << "," 
			 << v_results_sum[(*pv_possible_actions)[a]] << endl;
	}
	file.close();
}
