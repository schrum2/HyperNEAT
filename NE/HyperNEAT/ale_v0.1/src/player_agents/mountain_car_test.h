/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  mountain_car_test.h
 *
 *  Implementation of the Mountain Car problem. Used to make sure our 
 *  Sarsa Lambda implementation is correct *
 **************************************************************************** */

#ifndef MOUNTAIN_CAR_TEST_H
#define MOUNTAIN_CAR_TEST_H


#include "common_constants.h"
#include "rl_sarsa_lambda.h"
class OSystem;

class MountanCar {
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        MountanCar(OSystem* _osystem);
        
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        ~MountanCar();
        
        /* *********************************************************************
            Runs a single episode, and returns the total reward
         ******************************************************************** */
        float run_single_episode(void);

        /* *********************************************************************
            Runs the given number of episodes, and saves the recieved rewards
         ******************************************************************** */
        void run_experiment(int num_episodes);
        

        /* *********************************************************************
            Initilizes the speed and location to random values
         ******************************************************************** */
        void initialize_state_to_random(void);

        /* *********************************************************************
            Initilizes the speed and location to the bottom, with 0 velocity
         ******************************************************************** */
        void initialize_state_to_bottom(void);

        /* *********************************************************************
           Updates the current based on the action
         ******************************************************************** */
        void update_state(int action);
        
        /* *********************************************************************
           Returns true if we are in a terminating state
         ******************************************************************** */
        bool is_in_terminating_state(void);
        
        /* *********************************************************************
           Generates the feature vector map
         ******************************************************************** */
        void generate_feature_vec(void);
        float min_position; 
        float max_position;
        float goal_position;
        float max_velocity;
        double possible_actions[3];
        int num_actions;
        int num_observations;
        bool is_in_terminal;
        int max_num_steps;
        int curr_step_num;
        int num_tilings;
        int memory_size;
        float pos_scale;
        float vel_scale;
        float state_position;
        float state_velocity;
        FeatureMap* pv_curr_feature_map;
        IntVect* pv_num_nonzero_in_f;
        RLSarsaLambda* sarsa_lambda_solver;

};

#endif
