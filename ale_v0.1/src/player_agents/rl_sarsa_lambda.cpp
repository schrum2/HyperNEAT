/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  rl_sarsa_lambda.cpp
 *
 *  The implementation of a generic Gradient Descent Sarsa(lambda) solver.
 *  This can be plugged in various problems.
 *
 *  Note: large parts of this code is taken from Rich Sutton's Mountan Car 
 *        implementation
 **************************************************************************** */

#include <sstream>
// #include <math.h>
#include "../common/vector_matrix_tools.h"
#include "rl_sarsa_lambda.h"
#include "../emucore/OSystem.hxx"
#include "../common/export_tools.h"
#include "../common/random_tools.h"

/* *********************************************************************
    Constructor
 ******************************************************************** */
RLSarsaLambda::RLSarsaLambda(   OSystem* _osystem, int feature_vec_size, 
                                int num_actions, float alpha, 
                                int save_weights_freq) {
    p_osystem = _osystem;
    i_feature_vec_size = feature_vec_size;
    i_save_weights_freq = save_weights_freq;
    i_num_actions = num_actions;
	Settings& settings = p_osystem->settings();
    f_epsilon = settings.getFloat("epsilon", true);
    f_lambda = settings.getFloat("lambda", true);
    f_gamma = settings.getFloat("gamma", true);
    float eps_dim_start_r = settings.getFloat("epsilon_dim_start", true);
	int max_num_episodes = settings.getInt("max_num_episodes", true);
	if (eps_dim_start_r >= 0) {
		// diminish epsilon
		i_epsilon_dim_start = (int)(max_num_episodes * eps_dim_start_r);
		int num_diminishing_episodes = max_num_episodes - i_epsilon_dim_start;
		f_eps_dim_delta = f_epsilon / (float) num_diminishing_episodes;
		cout << "Will diminish epsilon " << f_eps_dim_delta << " per episode, "
			 << "after episode number: " << i_epsilon_dim_start << endl;
	} else {
		// don't diminish epsilon	
		i_epsilon_dim_start = max_num_episodes;
		f_eps_dim_delta = 0.0;
	}
	i_shrink_weights_frq = settings.getInt("shrink_weights_frq", true);
	if (i_shrink_weights_frq > 0) {
		f_shrink_weights_const = settings.getFloat("shrink_weights_const",true);
		s_shrink_weights_method=settings.getString("shrink_weights_method",true);
	} else {
		f_shrink_weights_const = 0.0;
		s_shrink_weights_method = "";
	}
    f_alpha = alpha;
    i_prev_action = -1; // Undefined
    i_curr_action = -1; // Undefined
    for (int i = 0; i < i_num_actions; i++) {
        v_Q.push_back(0);
    }
    pv_curr_features_map = NULL;
	pv_num_nonzero_in_f = NULL;
    i_episode_counter = 0;
	i_frame_counter = 0;
    float trace_vec_size_ratio = settings.getFloat("trace_vec_size_ratio",true);
    i_max_nonzero_traces = (int)(feature_vec_size * trace_vec_size_ratio);
    i_num_nonzero_traces = 0;
    f_minimum_trace = settings.getFloat("minimum_trace_value", true);
    b_optimistic_init = settings.getBool("optimistic_init", true);
    float weight_init = 0.0;
    if (b_optimistic_init) {
		cout << "Optimitic Initialization: all weights are initilized to"
			 << " 1.0/feature_vec_size" << endl;
        weight_init = 1.0 / (float)feature_vec_size;
    } 
    
    pv_weights = new FloatArr(i_feature_vec_size);
    pv_traces = new FloatArr(i_feature_vec_size);
    pv_nonzero_traces_inv_ind = new IntVect(i_feature_vec_size); 
    for (int i = 0; i < i_feature_vec_size; i++ ) {
        (*pv_weights)[i] = weight_init;
        (*pv_traces)[i] = 0.0;
        (*pv_nonzero_traces_inv_ind)[i] = -1;
    }
    pv_nonzero_traces_ind = new IntVect(i_max_nonzero_traces);
    for (int i = 0; i < i_max_nonzero_traces; i++) {
        (*pv_nonzero_traces_ind)[i] = -1;
    }
    i_prev_action = -1;
	b_normalize_fv = settings.getBool("normalize_feature_vector", true);
	if (b_normalize_fv) {
		cout << "Normalizing the Feature-Vec to sum up to 1.0" << endl;
	}
	// See if we are importing the weights
	string import_file = settings.getString("import_weights_file",  true);
	if (import_file.size() > 0) {
		cout << "Importing the weights vector from: << " << import_file << endl;
		import_array(pv_weights, import_file);
	}
}

/* *********************************************************************
    Deconstructor
 ******************************************************************** */
RLSarsaLambda::~RLSarsaLambda() {
    delete pv_weights;
    delete pv_traces;
    delete pv_nonzero_traces_ind;
    delete pv_nonzero_traces_inv_ind;
}
        
/* *********************************************************************
     This should be called at the beginijng of each episode
	 When forced_action_ind is not -1, we will choose the given action 
	 instead of choosing one based on epsilon-greedy
 * ****************************************************************** */
int RLSarsaLambda::episode_start(   FeatureMap* new_feature_map, 
                                    IntVect* num_nonzero_in_f, 
									int forced_action_ind) {
    DecayTraces(0.0); 
    pv_curr_features_map = new_feature_map;
    pv_num_nonzero_in_f = num_nonzero_in_f;
    computeActionValues(); 
	if (forced_action_ind == -1) {	
		i_prev_action = selectEpsilonGreedyAction();
	} else {
		i_prev_action = forced_action_ind;
	}
	return i_prev_action;
}

/* *********************************************************************
     One step of the main SARSA(lambda) algorithm:
        new_features_map: s'
        new_reward: r'
    Returns the aciton a' for state s'
	 Note: When forced_action_ind is not -1, we will choose the given action,
	 instead of choosing one based on epsilon-greedy
    // NOTE: pv_curr_features_map is a pointer to the actual feature-map
    // It is very likely that when new_feature_map is updated in other parts
    // of code, pv_curr_features_map also changes (since allocating and 
    // deleting a vector is ratehr expensive). Therefore, do not assume
    // that pv_curr_features_map is different from new_feature_map. 
 * ****************************************************************** */
int RLSarsaLambda::episode_step(FeatureMap* new_feature_map, 
                                IntVect* num_nonzero_in_f, float new_reward, 
								int forced_action_ind) {
    i_frame_counter++;
	assert(i_prev_action != -1);
	double delta = new_reward - v_Q[i_prev_action];	
	pv_curr_features_map = new_feature_map;
    pv_num_nonzero_in_f =  num_nonzero_in_f;
	computeActionValues();		 //new action values based on new observation
	if (forced_action_ind == -1) {	
		i_prev_action = selectEpsilonGreedyAction();
	} else {
		i_prev_action = forced_action_ind;
	}
	delta += (f_gamma * v_Q[i_prev_action]);
	if (!(delta - 1.0 < delta)) {
		cout << "Delta = inf.:(" << endl;
		cerr << "Delta = inf. :(" << endl;
		exit(-1);
	}
    updateWeights(delta);
    updateTraces();	
    computeActionValues(i_prev_action);
	return i_prev_action;
}
    
 
/* *********************************************************************
     This is called when  the episode ends. We manually insert the given
    value as the value of the final state of the episode
    'reward' is the reward the agent recieves as it transitions into the
    final state:  S[end-1] -> a -> reward -> S[end]
 * ****************************************************************** */
void RLSarsaLambda::episode_end(float reward, float value_of_final_state) {
    double delta = reward - v_Q[i_prev_action];
    delta += (f_gamma * value_of_final_state);  
    updateWeights(delta);
    i_episode_counter++;
	if ((f_eps_dim_delta > 0.0 || f_eps_dim_delta < 0.0 ) && 
		i_episode_counter > i_epsilon_dim_start) {
		// Diminishing Epsilon
		f_epsilon -= f_eps_dim_delta;
		if (f_epsilon < 0) {
			f_epsilon = 0;
		}
		cout << "epsilon updated to: " << f_epsilon << endl;
	}

	shrink_weights_vect(); // take care of shrinking the weights vector
	
    if ((i_save_weights_freq != 0) &&  
        (i_episode_counter % i_save_weights_freq == 0)) {
        cout << "Exporting the weights... ";
        ostringstream filename;
        filename << "exported_weights__episode_" << i_episode_counter << ".txt";
        export_weights(filename.str());
        cout << "done." << endl;
    }
}

/* *********************************************************************
   Compute all the action values from current activeFeatures and weights
 * ****************************************************************** */
void RLSarsaLambda::computeActionValues() {
	for (int a = 0; a < i_num_actions; a++) {
		v_Q[a] = 0;
		for (int j = 0; j < (*pv_num_nonzero_in_f)[a]; j++) {
			v_Q[a] += (*pv_weights)[(*pv_curr_features_map)[a][j]];
        }
		if (b_normalize_fv) {
			v_Q[a] = v_Q[a] / float((*pv_num_nonzero_in_f)[a]);
		}
	}
}

/* *********************************************************************
    Compute a particular action value from current activeFeatures and 
    weights
 * ****************************************************************** */
void RLSarsaLambda::computeActionValues(int a) {
	v_Q[a] = 0;
    for (int j = 0; j < (*pv_num_nonzero_in_f)[a]; j++) { 
		v_Q[a] += (*pv_weights)[(*pv_curr_features_map)[a][j]];
    }
	if (b_normalize_fv) {
		v_Q[a] = v_Q[a] / float((*pv_num_nonzero_in_f)[a]);
	}
}


/* *********************************************************************
    Returns index (action) of largest entry in QSA array, breaking ties 
    randomly
 * ****************************************************************** */
int RLSarsaLambda::argmax(FloatVect& QSA) {
	int best_action = 0;
    double best_value = QSA[0];
    int num_ties = 1;                    // actually the number of ties plus 1
    double value;
	
	for (int a = 1; a < i_num_actions; a++) {
		value = QSA[a];
        if (value >= best_value) {
            if (value > best_value) 
			{
				best_value = value;
				best_action = a;
			} else {
				num_ties++;
				if (0 == (int)(drand48()*num_ties)) {
					best_value = value;
					best_action = a;
				}
			}
        }
	}
    return best_action;

}
/* *********************************************************************
    Select an action according to epsilon greedy policy 
 * ****************************************************************** */
int RLSarsaLambda::selectEpsilonGreedyAction() {
	if(drand48() <= f_epsilon) {
		int rnd =  (int)(drand48()*(i_num_actions));
		return rnd;
	}
	else {
		return argmax(v_Q);
	}
}

/* *********************************************************************
    At the beggining of every episode, decay traces, clear action 
    traces and replace current trace
 * ****************************************************************** */
void RLSarsaLambda::updateTraces() {
	DecayTraces(f_gamma * f_lambda);                              
	for (int a = 0; a < i_num_actions; a++) { 
		if (a != i_prev_action) {
			for (int j = 0; j < (*pv_num_nonzero_in_f)[a]; j++) {
                ClearTrace((*pv_curr_features_map)[a][j]);
            }
        }
    }
    for (int j = 0; j < (*pv_num_nonzero_in_f)[i_prev_action]; j++) {
        // replace traces
        SetTrace((*pv_curr_features_map)[i_prev_action][j], 1.0); 
    }
}

/* *********************************************************************
    Update weights with nonzero traces using td-error
 * ****************************************************************** */
void RLSarsaLambda::updateWeights(double delta) {
	assert(f_alpha >= 0);
	double temp = f_alpha * delta;
	for (int i = 0; i < i_num_nonzero_traces; i++) { 
		int index = (*pv_nonzero_traces_ind)[i];
		(*pv_weights)[index] += (temp * (*pv_traces)[index]);
	}
    // print_largest_weight();
}

/* *********************************************************************
    This is for debugging only: prints the largest (+/-) value in 
    pv_weights and pv_traces
 * ****************************************************************** */
void RLSarsaLambda::print_largest_weight(void) {
    double largest_weight = 0;
    double smallest_weight = 0;
    double largest_trace = 0;
    double smallest_trace = 0;
	for (int i = 0; i < i_feature_vec_size; i++) { 
        if ((*pv_weights)[i] > largest_weight) {
            largest_weight = (*pv_weights)[i];
        }
        if ((*pv_weights)[i] < smallest_weight) {
            smallest_weight = (*pv_weights)[i];
        }
        if ((*pv_traces)[i] > largest_trace) {
            largest_trace = (*pv_traces)[i];
        }
        if ((*pv_traces)[i] < smallest_trace) {
            smallest_trace = (*pv_traces)[i];
        }
	}
    cout << "Largest Weight: " << largest_weight << " - ";
    cout << "Smallest Weight: " << smallest_weight << endl;
    cout << "Largest Trace: " << largest_trace << " - ";
    cout << "Smallest Trace: " << smallest_trace << endl;
    cout << "*********************************************************" << endl;
}

/* *********************************************************************
    clear or zero-out trace, if any, for given feature
 * ****************************************************************** */
void RLSarsaLambda::ClearTrace(int f) {
	if (!((*pv_traces)[f] == 0.0)) {
        ClearExistentTrace(f, (*pv_nonzero_traces_inv_ind)[f]);
    }
}

/* *********************************************************************
    clear trace at given location in list of nonzero-traces
 * ****************************************************************** */
void RLSarsaLambda::ClearExistentTrace(int f, int loc) { 
    (*pv_traces)[f] = 0.0;
    i_num_nonzero_traces--;
    (*pv_nonzero_traces_ind)[loc] = (*pv_nonzero_traces_ind)[i_num_nonzero_traces];
    (*pv_nonzero_traces_inv_ind)[(*pv_nonzero_traces_ind)[loc]] = loc;
}

/* *********************************************************************
    Decay all nonzero traces
 * ****************************************************************** */
void RLSarsaLambda::DecayTraces(float decay_rate) {
	for (int loc = i_num_nonzero_traces - 1; loc >= 0; loc--) { 
		int f = (*pv_nonzero_traces_ind)[loc];
        (*pv_traces)[f] *= decay_rate;
        if ((*pv_traces)[f] < f_minimum_trace) {
            ClearExistentTrace(f, loc);
        }
	}
}         

/* *********************************************************************
    set trace to given value
 * ****************************************************************** */
void RLSarsaLambda::SetTrace(int f, float new_trace_value) {
    if ((*pv_traces)[f] >= f_minimum_trace) {
        (*pv_traces)[f] = new_trace_value;         // trace already exists
    } else { 
        while (i_num_nonzero_traces >= i_max_nonzero_traces) {
            IncreaseMinTrace(); // ensure room for new trace
        }
        (*pv_traces)[f] = new_trace_value;
        (*pv_nonzero_traces_ind)[i_num_nonzero_traces] = f;
        (*pv_nonzero_traces_inv_ind)[f] = i_num_nonzero_traces;
        i_num_nonzero_traces++;
    }
}

/* *********************************************************************
    Try to make room for more traces by incrementing minimum_trace by 10%, 
    culling any traces that fall below the new minimum
 * ****************************************************************** */
void RLSarsaLambda::IncreaseMinTrace() {
    f_minimum_trace += (0.1 * f_minimum_trace);
    for (int loc = i_num_nonzero_traces - 1; loc >= 0; loc--) {
        int f = (*pv_nonzero_traces_ind)[loc];
        if ( (*pv_traces)[f] < f_minimum_trace) {
            ClearExistentTrace(f, loc);
        }
    }
}



/* *********************************************************************
   Saves the weights vector to file
 * ****************************************************************** */
void RLSarsaLambda::export_weights(const string& filename) {
    export_array(pv_weights, filename);
    for (unsigned int i = 0; i < 10; i++) {
        cout << (*pv_weights)[i] << ", ";
    }
    for (unsigned int i = 1; i <= 10; i++) {
        cout << (*pv_weights)[pv_weights->size() - i] << ", ";
    }
}

/* *********************************************************************
   Loads the weights vector from file
 * ****************************************************************** */
void RLSarsaLambda::import_weights(const string& filename) {
    import_array(pv_weights, filename);
    cout << "Weights Vector importd fromfile" + filename << endl;
    cout << "First 10 values: ";
    for (int i = 0; i < 10; i++) {
        cout << (*pv_weights)[i] << ", ";
    }
    cout << endl << "Last 10 values: ";
    for (int i = 1; i <= 10; i++) {
        cout << (*pv_weights)[pv_weights->size() - i] << ", ";
    }
}

/* *********************************************************************
	Takes care of shrinking the weights-vector (when it is enabled)
 * ****************************************************************** */
void RLSarsaLambda::shrink_weights_vect(void) {
	if ((i_shrink_weights_frq <= 0) || 
		(i_episode_counter % i_shrink_weights_frq != 0)) {
		return;
	}
	int num_non_zero = 0;
	for (unsigned int i = 0; i < pv_weights->size(); i++) {
		if ((*pv_weights)[i] != 0.0) {
			num_non_zero++;
		}
	}
	int num_vals_to_keep;
	if (s_shrink_weights_method == "sqr_root_num_frames") {
		num_vals_to_keep = (int)pow((float)i_frame_counter, 
									(float)(1.0/2.0));
		num_vals_to_keep = (int) (num_vals_to_keep * f_shrink_weights_const);
	} else if (s_shrink_weights_method == "linear_shrink") {
		num_vals_to_keep = (int) (num_non_zero * f_shrink_weights_const);
	} else if (s_shrink_weights_method == "const_shrink") {
		num_vals_to_keep = num_non_zero - (int)f_shrink_weights_const;
	} else {
		cerr << "Invalid value for shrink_weights_method" << endl;
		exit(-1);
	}
	
	cout << "Had " << num_non_zero << " non-zero values in w.";
	shrink_array(pv_weights, num_vals_to_keep);
	num_non_zero = 0;
	for (unsigned int i = 0; i < pv_weights->size(); i++) {
		if ((*pv_weights)[i] != 0) {
			num_non_zero++;
		}
	}
	cout << "Kept " << num_non_zero << " values." << endl;
}

/* *********************************************************************
	Generates an instance of one of the RLSarsaLambda subclasses, based 
	on the values of use_idbd and use_delta_bar_delta 
	Note: The caller is resposible for deleting the returned pointer
******************************************************************** */
RLSarsaLambda* RLSarsaLambda::generate_rl_sarsa_lambda_instance(
							OSystem* _osystem, int feature_vec_size,
							int num_actions) {
	Settings& settings = _osystem->settings();
    float alpha_multiplier = settings.getFloat("alpha_multiplier", true);
	float alpha = settings.getFloat("alpha", true);
	bool use_delta_bar_delta = settings.getBool("use_delta_bar_delta",true);
	bool use_idbd = settings.getBool("use_idbd",true);
	if (!use_idbd) {
		if (alpha == -1.0) {
			cout << "Alpha is determined automatically: ";
			alpha = 1.0 / ( (feature_vec_size / 2 ) * alpha_multiplier); 
		} else {
			cout << "Alpha is set to: " ;
		}
		cout << alpha << endl;
		assert (alpha >= 0);
	}
    int exp_w_frq = settings.getInt("export_weights_frq",true);
	if (use_idbd && use_delta_bar_delta) {
		cerr << "use_idbd and use_delta_bar_delta cannot both be true" << endl;
		exit(-1);
	}
	RLSarsaLambda* sarsa_lambda_solver = NULL;
	if (use_delta_bar_delta) {
		float eps_start = alpha;
		float k = (alpha / 2.0);
		float phi = 0.1;
		float theta = 0.1;
		cout << "Using Delta-Bar-Delta: " << endl 
			 << "\t eps_start = " << eps_start << endl
		     << "\t k = " << k << endl
			 << "\t phi = " << phi << endl
			 << "\t theta = " << theta << endl;
		sarsa_lambda_solver = new RLSarsaLambdaWithDeltaBarDelta(  _osystem, 
													feature_vec_size, 
													num_actions, 
													eps_start, k, phi, theta, 
													exp_w_frq);		
	} else if (use_idbd) {
		cout << "Using iDBD. " << endl ;
		sarsa_lambda_solver = new RLSarsaLambdaWithIDBD(_osystem, 
														feature_vec_size,  
														num_actions, 
														exp_w_frq);
	} else {
		cout << "Using static Alpha. " << endl ;
		sarsa_lambda_solver = new RLSarsaLambda(  _osystem, 
													feature_vec_size, 
													num_actions, alpha, 
													exp_w_frq);
	}
	return sarsa_lambda_solver;

}


/* *************************************************************************
* class RLSarsaLambdaWithIDBD
* 
*	Sarsa-Lambda with Incremental Delta-Bar-Delta
*  (This version doesn't need a alpha parameter for stepsize. Instead it
*  dynamically calculates a step-size for every bit of the feature vector
*  See "Adaptive Bias by Gradient Descent: An Incremental Version of 
*  Delta-Bar-Delta" by Rich Sutton, 1992
************************************************************************ */

/* *********************************************************************
	Constructor
 ******************************************************************** */
RLSarsaLambdaWithIDBD::RLSarsaLambdaWithIDBD(	OSystem* _osystem, 
						int feature_vec_size,  int num_actions,  
						int save_weights_freq) : 
						RLSarsaLambda(	_osystem, feature_vec_size, num_actions, 
										-1.0, save_weights_freq)  {
	Settings& settings = p_osystem->settings();
    f_theta = settings.getFloat("theta", true);
	cout << "Theta (iDBD meta-learning rate) = " << f_theta << endl;
    pv_h = new FloatArr(i_feature_vec_size);
    pv_beta = new FloatArr(i_feature_vec_size);
    for (int i = 0; i < i_feature_vec_size; i++ ) {
        (*pv_h)[i] = 0;
        (*pv_beta)[i] = 0;
    }
}

/* *********************************************************************
	Deconstructor
 ******************************************************************** */
RLSarsaLambdaWithIDBD::~RLSarsaLambdaWithIDBD() {
	delete pv_h;
	delete pv_beta;
}

/* *********************************************************************
	Update weights with nonzero traces using td-error
	TODO: We may want to bound Beta
 * ****************************************************************** */
void RLSarsaLambdaWithIDBD::updateWeights(double delta) {
	double alpha, decay;
	for (int i = 0; i < i_num_nonzero_traces; i++) {
		int index = (*pv_nonzero_traces_ind)[i];
		(*pv_beta)[index] += (	f_theta * delta * (*pv_traces)[index] * 
								(*pv_h)[index]	);	
		alpha = pow(M_E, (*pv_beta)[index]);
		decay = 1.0 - (alpha * (*pv_traces)[index] * (*pv_traces)[index]);
		if (decay < 0) {
			decay = 0;
		}
		decay = decay * (*pv_h)[index];
		(*pv_h)[index] = decay + alpha * delta * (*pv_traces)[index];
		(*pv_weights)[index] += (alpha * delta * (*pv_traces)[index]);
	}
}




/* *************************************************************************
*	class RLSarsaLambdaWithDeltaBarDelta:public RLSarsaLambda

*	Sarsa-Lambda with Delta-Bar-Delta (this is not Rich's iDBD, but the 
*	version previous to it by Jacob)
*  
*  See "Increased rates of Convergence Through Learning rate Adaptation" by 
*	Robert Jacobs, 1988
************************************************************************ */

/* *********************************************************************
	Constructor
 ******************************************************************** */
RLSarsaLambdaWithDeltaBarDelta::RLSarsaLambdaWithDeltaBarDelta(
		OSystem* _osystem, int feature_vec_size, 
		int num_actions, double eps_start, double k, double phi, double theta,
		int save_weights_freq) : 
		RLSarsaLambda(	_osystem, feature_vec_size, num_actions, 
		-1.0, save_weights_freq)  {
	f_k = k;
	f_phi = phi;
	f_theta = theta;
	pv_eps = new FloatArr(i_feature_vec_size);
    pv_delta_bar = new FloatArr(i_feature_vec_size);
    for (int i = 0; i < i_feature_vec_size; i++ ) {
        (*pv_eps)[i] = eps_start;
        (*pv_delta_bar)[i] = 0.0;
    }

}
/* *********************************************************************
	Deconstructor
 ******************************************************************** */
RLSarsaLambdaWithDeltaBarDelta::~RLSarsaLambdaWithDeltaBarDelta() {
	delete pv_eps;
	delete pv_delta_bar;
}

/* *********************************************************************
	Update weights with nonzero traces using td-error
 * ****************************************************************** */
void RLSarsaLambdaWithDeltaBarDelta::updateWeights(double delta) {
	double new_delta, delta_bar_delta;
	for (int i = 0; i < i_num_nonzero_traces; i++) {
		int index = (*pv_nonzero_traces_ind)[i];
		// 1- Compute delta 
		new_delta =  delta * (*pv_traces)[index];
		
		// 2- Update epsilon
		delta_bar_delta = new_delta * (*pv_delta_bar)[index];
		if (delta_bar_delta > 0) {
			(*pv_eps)[index] += f_k;
		} else if (delta_bar_delta < 0) {
			(*pv_eps)[index] -= f_phi * (*pv_eps)[index];
		}
		
		// 3- Update delta-bar
		(*pv_delta_bar)[index] = ((1 - f_theta) * new_delta) + 
							 (f_theta * (*pv_delta_bar)[index]);
		// 4- Update the weights vector
		(*pv_weights)[index] += ((*pv_eps)[index] * new_delta);
	}

}


