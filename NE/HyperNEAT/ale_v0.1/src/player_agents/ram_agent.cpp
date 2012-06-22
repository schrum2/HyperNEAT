/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  ram_agent.cpp
 *
 *  The implementation of the RAMAgent class, which is a generic player 
 *  agent that learns to play, based on looking at the Console RAM content
 **************************************************************************** */

#include <sstream>
#include "ram_agent.h"
#include "../common/export_tools.h"  //TMP

RAMAgent::RAMAgent(GameSettings* _game_settings, OSystem* _osystem) :
    PlayerAgent(_game_settings, _osystem) {
    i_ram_bits_length = RAM_LENGTH * 8;
    i_base_length = i_ram_bits_length +      // = 1024 + 523776
                    (i_ram_bits_length * (i_ram_bits_length / 2) - 
                    (i_ram_bits_length / 2));
    assert (i_base_length == 1024 + 523776);
    i_full_feature_vec_length = i_base_length * i_num_actions;
	cout << "Full Feature-Vector Length: " << i_full_feature_vec_length << endl;
	p_sarsa_lambda_solver = RLSarsaLambda::generate_rl_sarsa_lambda_instance(
						p_osystem, i_full_feature_vec_length, i_num_actions);

    pv_curr_feature_map  = new FeatureMap();
    pv_num_nonzero_in_f = new IntVect();
    for (int i = 0; i < i_num_actions; i++) {
        IntArr feature_vec = IntArr(-1, i_base_length);
        pv_curr_feature_map->push_back(feature_vec);
        pv_num_nonzero_in_f->push_back(0);
    }
    pv_tmp_fv_first_part = new IntArr(-1, RAM_LENGTH * 8);
    pv_subvector_positions = new IntVect;
    for (int a = 0; a < i_num_actions; a++) {
        pv_subvector_positions->push_back(a * i_base_length);
    }

	b_end_episode_with_reward = p_osystem->settings().getBool(
											"end_episode_with_reward", true);
}

RAMAgent::~RAMAgent() {
    delete p_sarsa_lambda_solver;
    delete pv_curr_feature_map;
    delete pv_num_nonzero_in_f;
    delete pv_tmp_fv_first_part;
    delete pv_subvector_positions;

}
        
/* *********************************************************************
    Selects the enxt action, based on the content of the ram
 ******************************************************************** */
Action RAMAgent::agent_step(const IntMatrix* screen_matrix, 
                            const IntVect* console_ram, 
							int frame_number) {
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
Action RAMAgent::on_start_of_game(void) {
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
void RAMAgent::on_end_of_game(void) {
    PlayerAgent::on_end_of_game();
    cout << "V(end) = " << f_curr_reward << endl;
	p_sarsa_lambda_solver->episode_end(f_curr_reward, f_curr_reward);
}



/* *********************************************************************
    Generates a feature vector from the content of the ram.
    We put the bit values in the RAM (128 * 8 = 1024) and their cross
    products (1024 * 1024 / 2 - 512 = 523776)
    Since we are required to return a feature_map 
    (i.e. a seperate feature vector for each action), what we do next is 
    we generate a vector of size base_size * num_actions which is all 
    zero, except the portion dedicated for action a.
 ******************************************************************** */
void RAMAgent::generate_feature_vec(void) {
    for (int a = 0; a < i_num_actions; a++) {
        (*pv_num_nonzero_in_f)[a] = 0;
    }
    int full_vect_index = 0;
    int byte_val, bit_val;
    // Get the bit values of ram content into our temp array,
    // Also, generate the first part of the feature-vector
    for (int i = 0; i < RAM_LENGTH; i++) {
        byte_val = (*pv_curr_console_ram)[i];
        for (int k = 7; k >= 0; k--) {
            bit_val = (byte_val >> k) & 1;
            (*pv_tmp_fv_first_part)[full_vect_index] = bit_val;
            if (bit_val == 1) {
                add_one_index_to_feature_map(full_vect_index);
            }
            full_vect_index++;
        }
    }
		
   // Now we'll get the crossproduct of all the bits
    for (int i = 0; i < i_ram_bits_length; i++) {
//      Possible major optimization:
//        if ((*pv_tmp_fv_first_part)[i]  == 0) {
//            full_vect_index += (i_ram_bits_length - (i + 1));
//            continue;
//        }
        for (int j = i + 1; j < i_ram_bits_length; j++) {
            if ((*pv_tmp_fv_first_part)[i]  == 1 &&
                (*pv_tmp_fv_first_part)[j] == 1) { 
                add_one_index_to_feature_map(full_vect_index);
            }
            full_vect_index++;
        }
    }
    assert(full_vect_index == i_base_length);
}


