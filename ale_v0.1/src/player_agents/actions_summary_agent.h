/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  actions_summary_agent.h
 *
 * The implementation of the ActionSummaryAgent class, which generates a summary
 * of taking each action in the game for num_episodes_to_avg_results episodes
 **************************************************************************** */
 
#ifndef ACTION_SUMMARY_AGENT_H
#define ACTION_SUMMARY_AGENT_H

#include "common_constants.h"
#include "player_agent.h"

class ActionSummaryAgent : public PlayerAgent {

    public:
        ActionSummaryAgent(GameSettings* _game_settings, OSystem* _osystem);
        
        /* *********************************************************************
            Returns a random action from the set of possible actions
         ******************************************************************** */
        virtual Action agent_step(  const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_number);
	
		/* *********************************************************************
            This method is called when the game ends. The superclass 
            implementation takes care of counting number of episodes, and 
            saving the reward history. Here we check to see if we need to 
			swtch to the next action, and also update the episode counter and
			v_act_epis_rewards
        ******************************************************************** */
        virtual void on_end_of_game(void);
		
	protected:
		/* *********************************************************************
            Saves the summary results to a text file
        ******************************************************************** */	
		void save_results_summary(void);
		
		int i_curr_action_index;	// The action we are curently returning
									// -1 means Random
		string str_current_action;	// Name of the current action
		int i_act_episode_counter;	// How many episodes we have used this action
		int i_max_episode_per_act;	// Maximum episodes for each action
		FloatVect v_act_epis_rewards;	// The rewards per episode for curr act
		FloatVect v_results_sum;	// v_results_sum[i] is the average 
									// reward/episode fr action i
};

#endif
