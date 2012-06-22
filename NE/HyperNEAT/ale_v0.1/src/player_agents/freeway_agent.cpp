/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  freeway_agent.cpp
 *
 *  The implementation of the FreewayAgent class, a subclass of ClassAgent,
 *  which is specifically desinged to solve the game Freeway 
 **************************************************************************** */

#include <sstream>
#include "freeway_agent.h"
#include "class_agent.h"
#include "../common/export_tools.h"
#include "../control/game_controller.h"
#include "tiles2.h"
#include "../common/random_tools.h"
#include "../common/misc_tools.h"

FreewayAgent::FreewayAgent(GameSettings* _game_settings, OSystem* _osystem) :
ClassAgent(_game_settings, _osystem) {
	// TMP: For now, the feature-vec size should be assigned in the ClassAgent
	// (too much design hassel to move the vector initializations here)
	// I am just double checking that it is indeed set correctly
	int curr_full_feature_vec_length = i_full_feature_vec_length;
	calc_feature_vec_size();
	if (i_full_feature_vec_length != curr_full_feature_vec_length) {
		cout << "Feature-vec size not set correctly for Freeway agent." << endl;
		cerr << "Feature-vec size not set correctly for Freeway agent." << endl;
		exit(-1);
	}
	// Especial optimistic-initialization for Freeway:
	bool optimistic_init = p_osystem->settings().getBool("optimistic_init", true);
	if (optimistic_init) {
		float weight_init = 1.0 / (float)(i_num_tilings * 11);
		cout << "Setting all weights to " << weight_init << endl;
		for (int i = 0; i < i_full_feature_vec_length; i++ ) {
			(*(p_sarsa_lambda_solver->get_weights()))[i] = weight_init;
		}
	}

	pm_debug_best_val = new FloatMatrix();
	int cars_y[] = {175, 160, 145, 128, 111, 96, 80, 64, 47, 32};
    v_debug_car_y_vals = vector<int>(cars_y, cars_y + 10);
	int cars_color_ind[] = {26, 216, 68, 136, 36, 130, 74, 18, 220, 66};
	v_debug_car_color_ind = vector<int>(cars_color_ind, cars_color_ind + 10);
	float cars_vel[] = {0.78, 0.96, 1.26, 1.93, 3.85, -3.64, -1.92, 
						-1.28, -0.96, -0.76};
	v_debug_car_vel = vector<double>(cars_vel, cars_vel + 10); 
	v_debug_car_x_vals.clear();
	for (int c = 0; c < 10; c ++) {
		v_debug_car_x_vals.push_back(-1);
	}
}


FreewayAgent::~FreewayAgent() {
	delete pm_debug_best_val;
}

/* *********************************************************************
	Calculates the size of the full feature-vector
 ******************************************************************** */
void FreewayAgent::calc_feature_vec_size(void) {
	i_full_feature_vec_length = i_mem_size_2d; // Chicken Only
	i_full_feature_vec_length += (i_mem_size_2d * 4); // Chick-Car 
	i_full_feature_vec_length++;
}

/* *********************************************************************
    Returns an action from the set of possible actions.
    Runs one step of the Sarsa-Lambda algorithm
 ******************************************************************** */
Action FreewayAgent::agent_step(  const IntMatrix* screen_matrix, 
                                const IntVect* console_ram, 
								int frame_number) {
    Action return_action = ClassAgent::agent_step(screen_matrix, console_ram,
													frame_number);
	if (i_frame_counter % 10 == 0) {
		plot_best_value_per_state();
	}
	return return_action;
}
	
/* *********************************************************************
    This method is called when the game ends. 
 ******************************************************************** */
void FreewayAgent::on_end_of_game(void) {
    ClassAgent::on_end_of_game();
	if (false) {
		assert (pm_debug_best_val->size() > 0);
		FloatVect new_row2;
		for (int j = 0; j < (*pm_debug_best_val)[0].size(); j ++) {
			new_row2.push_back(1314.0);
		}
		pm_debug_best_val->push_back(new_row2);
		pm_debug_best_val->push_back(new_row2);			
			
		if(i_episode_counter % 10 == 0) {
			ostringstream filename2;
			filename2 << "state_values__episode_" << i_episode_counter << ".png";
			FloatMatrix* tmp_matrix = new FloatMatrix(*pm_debug_best_val);
			p_osystem->p_export_screen->save_heat_plot(tmp_matrix, filename2.str());
			delete tmp_matrix;
		}
	}
}

/* *********************************************************************
    Generates a feature vector by tile-coding the absolute chicken position
	as well as relative position/velocity of chicken/car
 ******************************************************************** */
void FreewayAgent::generate_feature_vec(void) {
    for (int a = 0; a < i_num_actions; a++) {
        int start_ind = 0;
        (*pv_num_nonzero_in_f)[a] = 0;
		// Chicken Pos Only!
		start_ind += i_num_actions;
		int chicken_y = get_chicken_y(pm_curr_screen_matrix);
		if (chicken_y == -1) {
			ostringstream filename;
			filename << "chick_not_found__" << i_frame_counter << ".png"; 
			p_osystem->p_export_screen->save_png(pm_curr_screen_matrix, filename.str());
		}
		assert(chicken_y != -1);
		BlobObject chick_obj(48, chicken_y);
		float chick_scaled_x, chick_scaled_y;
		get_scaled_position(chick_obj, chick_scaled_x, chick_scaled_y);
		pf_tmp_float_arr[0] = chick_scaled_y;
		GetTiles(pv_tmp_abs_ind, i_num_tilings, i_mem_size_2d,
				pf_tmp_float_arr, 1, a); 
		(*pv_tmp_abs_ind) += start_ind;
		append_to_feature_vec(pv_tmp_abs_ind, i_num_tilings, a);
		start_ind += i_mem_size_2d;
		
		// Chicken-Car positions
		get_cars_x(pm_curr_screen_matrix);
		for (int c = 0; c < 10; c++) {
			BlobObject car_obj(v_debug_car_x_vals[c], v_debug_car_y_vals[c], 
								v_debug_car_vel[c], 0.0);
			float x_rel_scaled, y_rel_scaled;
			get_scaled_relative_position(chick_obj, car_obj, 
										x_rel_scaled, y_rel_scaled);
			bool is_approaching_x, is_approaching_y;
			get_binary_relative_velocity(chick_obj, car_obj, 
											is_approaching_x, is_approaching_y);
			int vel_offset = 0;
			if (is_approaching_x) {
				vel_offset += 1;
			} 
			if (is_approaching_y) {
				vel_offset += 2;
			}
			assert(vel_offset >= 0 && vel_offset <= 3);
			vel_offset = vel_offset * i_mem_size_2d;
			
//			float scaled_rel_vel_x, scaled_rel_vel_y;
//			get_scaled_relative_velocity(chick_obj, car_obj, 
//								scaled_rel_vel_x, scaled_rel_vel_y);
			pf_tmp_float_arr[0] = x_rel_scaled;
			pf_tmp_float_arr[1] = y_rel_scaled;
//			pf_tmp_float_arr[2] = scaled_rel_vel_x;
//			pf_tmp_float_arr[3] = scaled_rel_vel_y;
//			unsigned int offset = start_ind;
//			GetTiles(pv_tmp_rel_ind, i_num_tilings, i_mem_size_4d,
//					pf_tmp_float_arr, 4, a, offset); 
			unsigned int offset = start_ind + vel_offset;
			GetTiles(pv_tmp_rel_ind, i_num_tilings, i_mem_size_2d,
					pf_tmp_float_arr, 2, a, offset); 
//			(*pv_tmp_rel_ind) += (start_ind + vel_offset);
			append_to_feature_vec(pv_tmp_rel_ind, i_num_tilings, a);
		}
		start_ind += (4 * i_mem_size_2d); 
//		start_ind += i_mem_size_4d; 
    }
}



/* *********************************************************************
	Used for Debugging Freeway.
	Plots the best value based on our current w, for different values
	of the chicken position.
 ******************************************************************** */
void FreewayAgent::plot_best_value_per_state(void) {
	FloatVect new_row;
	FloatVect v_Q;
	for (int a = 0; a < i_num_actions; a++) {
		v_Q.push_back(0);
	}		
	
	for (int chicken_y = 23; chicken_y < 192;) {
		BlobObject obj( 47, chicken_y);
		// Generate a feature map
		for (int a = 0; a < i_num_actions; a++) {
			int start_ind = 0;
			(*pv_num_nonzero_in_f)[a] = 0;
			start_ind += i_num_actions;
			float scaled_x, scaled_y;
			get_scaled_position(obj, scaled_x, scaled_y);
			pf_tmp_float_arr[0] = scaled_y;
			GetTiles(pv_tmp_abs_ind, i_num_tilings, i_mem_size_2d,
				pf_tmp_float_arr, 1, a);
			(*pv_tmp_abs_ind) += start_ind;
			append_to_feature_vec(pv_tmp_abs_ind, i_num_tilings, a);
		}
		
		// Calculate action values
		for (int a = 0; a < i_num_actions; a++) {
			v_Q[a] = 0;
			for (int j = 0; j < (*pv_num_nonzero_in_f)[a]; j++) {
				v_Q[a] += (*p_sarsa_lambda_solver->get_weights())[(*pv_curr_feature_map)[a][j]];
			}
		}
		// find the maximum
		double best_value = v_Q[0];	
		for (int a = 1; a < i_num_actions; a++) {
			if (v_Q[a] > best_value) {
				best_value = v_Q[a];
			}
		}
		new_row.push_back(best_value);
		
		chicken_y += 4;
	}
	
	// Mark the current position of the chicken 
	if ((*pv_curr_cls_inst_map)[2].size() > 0) {
		BlobObject& obj = (*pv_curr_cls_inst_map)[2][0];
		int chicken_real_y = obj.i_center_y;
		if (chicken_real_y >= 23 && chicken_real_y <= 191) {
			chicken_real_y -= 23;
			chicken_real_y = chicken_real_y / 4;
			new_row[chicken_real_y] = 1313.0;
		} else {
			new_row[new_row.size() - 1] = 1314.0;
		}
	} else {
		new_row[new_row.size() - 1] = 1314.0;
	}
	// Calculate the value of no chicken
	for (int a = 0; a < i_num_actions; a++) {
		(*pv_num_nonzero_in_f)[a] = 0;
		int one_ind = a;
		add_one_index_to_feature_map(one_ind, a);
		v_Q[a] = 0;
		for (int j = 0; j < (*pv_num_nonzero_in_f)[a]; j++) {
			v_Q[a] += (*p_sarsa_lambda_solver->get_weights())[(*pv_curr_feature_map)[a][j]];
		}
	}
	double best_value = v_Q[0];	
	for (int a = 1; a < i_num_actions; a++) {
		if (v_Q[a] > best_value) {
			best_value = v_Q[a];
		}
	}
	pm_debug_best_val->push_back(new_row);
}


/* *********************************************************************
	Used for Debugging Freeway.
	Manually extracts the current y-value of the chicken
 ******************************************************************** */
int FreewayAgent::get_chicken_y(const IntMatrix* pv_screen) {
	int chicken_color_ind = 30;
	int chicken_x_val = 48;
	int yellow_line_y_val1 = 102;
	int yellow_line_y_val2 = 104;
	for (int i = 0; i < i_screen_height; i++) {
		for (int x = -2; x <= 2; x++) {
			if ((*pv_screen)[i][chicken_x_val + x] == chicken_color_ind &&
				i != yellow_line_y_val1 &&
				i != yellow_line_y_val2) {
				return i;
			}
		}
	}
	return -1; // cannot find the chicken
}

/* *********************************************************************
	Used for Debugging Freeway.
	Manually extracts the current x-values of the cars
 ******************************************************************** */
void FreewayAgent::get_cars_x(const IntMatrix* pm_screen) {
	for (int c = 0; c < 10; c++) {
		int car_y = v_debug_car_y_vals[c];
		int color_ind = v_debug_car_color_ind[c];
		bool car_found = false;
		for (int j = 0; j < i_screen_width; j++) {
			for (int y = -2; y <= 2; y++) {
				if ((*pm_screen)[car_y + y][j] == color_ind) {
					car_found = true;
					v_debug_car_x_vals[c] = j;
					break;
				}
			}
		}
		if (!car_found) {
			v_debug_car_x_vals[c] = 0;
		}
	}
}
