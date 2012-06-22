/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
*  mountain_car_test.cpp
*
*  Implementation of the Mountain Car problem. Used to make sure our 
*  Sarsa Lambda implementation is correct *
**************************************************************************** */
#include <math.h>
#include "../emucore/OSystem.hxx"
#include "mountain_car_test.h"
#include "tiles2.h"
#include "../common/export_tools.h"

/* *********************************************************************
    Constructor
 ******************************************************************** */
MountanCar::MountanCar(OSystem* _osystem) {
    min_position       = -1.2;
    max_position       = 0.6;
    goal_position      = 0.5;
    max_velocity       = 0.07;
    possible_actions[0]= -1.0;
    possible_actions[1]= 0.0;
    possible_actions[2]= 1.0;
    num_actions        = 3;
    num_observations   = 2; 
    is_in_terminal     = false;
    max_num_steps      = 1000000;
    curr_step_num      = 0;
    // Tiling Properties
    num_tilings        = 14;
    memory_size        = 10000;
    pos_scale          = 1.7 / 8.0;
    vel_scale          = 0.14 / 8.0;
    state_position     = 0.0;
    state_velocity     = 0.0;
    pv_curr_feature_map = new FeatureMap();
    pv_num_nonzero_in_f = new IntVect();
    for (int i = 0; i < num_actions; i++) {
        IntArr feature_vec = IntArr(-1, num_tilings);
        pv_curr_feature_map->push_back(feature_vec);
        pv_num_nonzero_in_f->push_back(num_tilings);
    }
    float alpha = 0.5 / num_tilings;
    _osystem->settings().setFloat("epsilon", 0.1);
    _osystem->settings().setFloat("lambda", 0.95);
    _osystem->settings().setFloat("gamma", 1.0);
	_osystem->settings().setFloat("alpha", alpha);
    _osystem->settings().setBool("optimistic_init", true);
    _osystem->settings().setFloat("trace_vec_size_ratio", 0.1);
    _osystem->settings().setFloat("minimum_trace_value", 0.01);
	sarsa_lambda_solver = RLSarsaLambda::generate_rl_sarsa_lambda_instance(
										_osystem, memory_size, num_actions);
	
}

/* *********************************************************************
    Deconstructor
 ******************************************************************** */
MountanCar::~MountanCar() {
    delete pv_curr_feature_map;
    delete pv_num_nonzero_in_f;
    delete sarsa_lambda_solver;
}

/* *********************************************************************
    Runs a single episode, and returns the total reward
 ******************************************************************** */
float MountanCar::run_single_episode(void) {
    initialize_state_to_random();
    // initialize_state_to_bottom();
    cout << "Initial Pos: " << state_position << " - ";
    cout << "Initial Vel: "<< state_velocity << " - ";
    float total_reward = 0.0;
    generate_feature_vec();
    int next_action_ind = sarsa_lambda_solver->episode_start(
                                    pv_curr_feature_map, pv_num_nonzero_in_f);
    double next_action = possible_actions[next_action_ind];
    bool episode_terminated = false;
    curr_step_num = 0;
    while(true) {
        curr_step_num++;
        // 2- Take the action
        update_state(next_action);
        episode_terminated = is_in_terminating_state();
        if (episode_terminated) {
            sarsa_lambda_solver->episode_end(-1.0, -1.0);
            break;
        }
        // 3- observe the new state/reward
        generate_feature_vec();
        float reward = -1.0; // Reward is always -1
        total_reward += reward;
        // 4- Choose the next action
        next_action_ind = sarsa_lambda_solver->episode_step(
                            pv_curr_feature_map, pv_num_nonzero_in_f, reward);
        next_action = possible_actions[next_action_ind];
    }
    return total_reward;
}

/* *********************************************************************
    Runs the given number of episodes, and saves the recieved rewards
 ******************************************************************** */
void MountanCar::run_experiment(int num_episodes) {
    FloatVect v_rewards;
    for (int i = 0; i < num_episodes; i++) {
        cout << "Epsidoe " << i << ": ";
        float total_reward = run_single_episode();
        v_rewards.push_back(total_reward);
        cout << "Sum Reward = " << total_reward << " \n";
    }
    export_vector(&v_rewards, "mountain_car_rewards.txt");
}

/* *********************************************************************
    Initilizes the speed and location to random values
 ******************************************************************** */
void MountanCar::initialize_state_to_random(void) {
    state_position = (drand48()*(goal_position + fabs(min_position)) + min_position);
    state_velocity = (drand48()*(max_velocity*2) - max_velocity);
}

/* *********************************************************************
    Initilizes the speed and location to the bottom, with 0 velocity
 ******************************************************************** */
void MountanCar::initialize_state_to_bottom(void) {
    state_position = -M_PI/6.0;
    state_velocity = 0.0; 
}


/* *********************************************************************
   Updates the current based on the action
 ******************************************************************** */
void MountanCar::update_state(int action) {
    // Updates the current based on the the action
    // 1- Update the velocity
    state_velocity += ((action * 0.001) + cos(3.0 * state_position) * (-0.0025));
    if (state_velocity > max_velocity) {
        state_velocity = max_velocity;
    }
    if (state_velocity < -max_velocity) {
        state_velocity = -max_velocity;
    }

    // 2- Update the position
    state_position += state_velocity;
    if (state_position > max_position) {
        state_position = max_position;
    }
	if (state_position < min_position) { 
        state_position = min_position;
    }
    if (state_position == min_position && state_velocity < 0) { 
        state_velocity = 0;
    }
}

/* *********************************************************************
   Returns true if we are in a terminating state
 ******************************************************************** */
bool MountanCar::is_in_terminating_state(void) {
    if  (   state_position >= goal_position ||
            curr_step_num > max_num_steps) {
        return true;
    } else {
        return false;
    }
}

/* *********************************************************************
   Generates the feature vector map
 ******************************************************************** */
void MountanCar::generate_feature_vec(void) {
    float float_vec[2];
    float_vec[0] = state_position / pos_scale;
    float_vec[1] = state_velocity /  vel_scale;
	
    for (int a = 0; a < num_actions; a++) {
//        int activeFeatures[num_tilings];
        GetTiles(&((*pv_curr_feature_map)[a]), num_tilings, memory_size, 
                float_vec, num_observations, a);
//        for (int i = 0; i < num_tilings; i++) {
//            (*pv_curr_feature_map)[a][i] = activeFeatures[i];
//        }
    }
}
