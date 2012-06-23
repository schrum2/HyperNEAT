/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  ram_agent.h
 *
 *  The implementation of the RAMAgent class, which is a generic player 
 *  agent that learns to play, based on looking at the Console RAM content
 **************************************************************************** */

#ifndef RAM_AGENT_H
#define RAM_AGENT_H

#include "common_constants.h"
#include "player_agent.h"
#include "rl_sarsa_lambda.h"

class RAMAgent : public PlayerAgent {
    /* *************************************************************************
        This isa generic player agent that learns to play, based on looking at 
        the Console RAM content
        
        Instance variabls:
        - pv_curr_feature_map   pv_curr_feature_map[a] is the feature-vector 
                                for action a.
        - pv_num_nonzero_in_f   Number of non-zero values in each feature-vector 
        - pv_tmp_fv_first_part  An array containing the first part of the 
                                feature-vector (this is only used internally)
        - p_sarsa_lambda_solver Pointer to the SARSA-Lambda solver object
        - i_ram_bits_length     Number of bits in RAm (128 * 8 = 1024)
        - i_base_length         Number of bits required to keep 128 * 8 = 1024
                                bits of RAm content, plus the cross-product
                                of these bits. The length of a full 
                                feature-vector will be :
                                i_num_actions * i_base_length
        - i_full_feature_vec_length     Number of bits in the *full* feature
                                vector. Note that the feature-map that we 
                                generate only holds the one-indecies of this 
                                vector
        - alpha_multiplier  alpha = 1.0 / 
                               (num_ones_in_vector * rl_params.alpha_multiplier)
        - pv_subvector_positions This is only for optimization:
                                pv_subvector_positions[a] = a * i_base_length
		- b_end_episode_with_reward  When true, episode ends after each reward.
    ************************************************************************* */

    public:
        
        RAMAgent(GameSettings* _game_settings, OSystem* _osystem);
        virtual ~RAMAgent();
        
        /* *********************************************************************
            Returns a random action from the set of possible actions
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
            Generates a feature vector from the content of the ram.
            We put the bit values in the RAM (128 * 8 = 1024) and their cross
            products (1024 * 1024 / 2 - 512 = 523776)
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

        FeatureMap* pv_curr_feature_map;
        IntVect* pv_num_nonzero_in_f;
        RLSarsaLambda* p_sarsa_lambda_solver;
        IntArr* pv_tmp_fv_first_part;
        int i_ram_bits_length;
        int i_base_length;
        int i_full_feature_vec_length;
        float f_alpha_multiplier;
        IntVect* pv_subvector_positions;
		bool b_end_episode_with_reward;
};

#endif




