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


#ifndef RL_SARSA_LAMBDA_H
#define RL_SARSA_LAMBDA_H

#include "common_constants.h"
class OSystem;

class RLSarsaLambda {
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        RLSarsaLambda(OSystem* _osystem, int feature_vec_size, 
                        int num_actions, float alpha, int save_weights_freq) ;
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~RLSarsaLambda();
        
        /* *********************************************************************
             This should be called at the beginijng of each episode
			 When forced action is not UNDEFINED, we will choose the given action 
			instead of choosing one based on epsilon-greedy
         * ****************************************************************** */
        virtual int episode_start(  FeatureMap* new_feature_map, 
                            IntVect* num_nonzero_in_f, 
							int forced_action_ind = -1);

        /* *********************************************************************
             One step of the main SARSA(lambda) algorithm:
                new_features_map: s'
                new_reward: r'
            Returns the aciton a' for state s'
			When forced action is not UNDEFINED, we will choose the given action 
			instead of choosing one based on epsilon-greedy
         * ****************************************************************** */
        virtual int episode_step(   FeatureMap* new_feature_map, 
                            IntVect* num_nonzero_in_f, float new_reward, 
							int forced_action_ind = -1);
         
        /* *********************************************************************
             This is called when  the episode ends. We manually insert the given
            value as the value of the final state of the episode
            'reward' is the reward the agent recieves as it transitions into the
            final state:  S[end-1] -> a -> reward -> S[end]
         * ****************************************************************** */
        virtual void episode_end(float reward, float value_of_final_state);

        /* *********************************************************************
           Sets a new alpha (step-size) value
         * ****************************************************************** */
        void inline set_alpha(double new_alpha) {
            f_alpha = new_alpha;
        };
        
        
        /* *********************************************************************
           Saves the weights vector to file
         * ****************************************************************** */
        virtual void export_weights(const string& filename);

        /* *********************************************************************
           Loads the weights vector from file
         * ****************************************************************** */
        virtual void import_weights(const string& filename);

        /* *********************************************************************
           Accessor method
         * ****************************************************************** */
        inline FloatArr* get_weights(void) {return pv_weights;};

        /* *********************************************************************
            Generates an instance of one of the RLSarsaLambda subclasses, based 
            on the values of use_idbd and use_delta_bar_delta 
            Note: The caller is resposible for deleting the returned pointer
        ******************************************************************** */
        static RLSarsaLambda* generate_rl_sarsa_lambda_instance(
									OSystem* _osystem, int feature_vec_size,
									int num_actions);
        
    protected:
        /* *********************************************************************
           Compute all the action values from current activeFeatures and weights
         * ****************************************************************** */
        virtual void computeActionValues();                         

        /* *********************************************************************
            Compute a particular action value from current activeFeatures and 
            weights
         * ****************************************************************** */
        virtual void computeActionValues(int a);                              


        /* *********************************************************************
            Returns index (action) of largest entry in QSA array, breaking ties 
            randomly
         * ****************************************************************** */
        virtual int argmax(FloatVect& QSA);            

        /* *********************************************************************
            Select an action according to epsilon greedy policy 
         * ****************************************************************** */
        virtual int selectEpsilonGreedyAction();

        /* *********************************************************************
            At the beggining of every episode, decay traces, clear action 
            traces and replace current trace
         * ****************************************************************** */
        virtual void updateTraces();

        /* *********************************************************************
            Update weights with nonzero traces using td-error
         * ****************************************************************** */
        virtual void updateWeights(double delta);

        /* *********************************************************************
            This is for debugging only: prints the largest (+/-) value in 
            pv_weights and pv_traces
         * ****************************************************************** */
        void print_largest_weight(void);

        /* *********************************************************************
            clear or zero-out trace, if any, for given feature
         * ****************************************************************** */
        void ClearTrace(int f);                          

        /* *********************************************************************
            clear trace at given location in list of nonzero-traces
         * ****************************************************************** */
        void ClearExistentTrace(int f, int loc);         

        /* *********************************************************************
            decay all nonzero traces
         * ****************************************************************** */
        void DecayTraces(float decay_rate);              

        /* *********************************************************************
            set trace to given value
         * ****************************************************************** */
        void SetTrace(int f, float new_trace_value);     

        /* *********************************************************************
            increase minimal trace value, forcing more to 0, making room for new 
            ones
         * ****************************************************************** */
        void IncreaseMinTrace();                         

        /* *********************************************************************
            Takes care of shrinking the weights-vector (when it is enabled)
         * ****************************************************************** */
        void shrink_weights_vect(void);                         

        OSystem* p_osystem;       // Pointer to the OSystem object
        double f_epsilon;          // probability of random action
        double f_alpha;            // step size parameter
        double f_lambda;           // trace-decay parameter
        double f_gamma;            // discount-rate parameter
        bool b_optimistic_init;   // when true, we do optimitic initialization
		bool b_normalize_fv;	  // When true feature-vector will be normalized
        int i_num_actions;        // number of actions
        int i_feature_vec_size;   // maximum number of features
        FloatArr* pv_weights;     // vector of feature weights
        FloatArr* pv_traces;      // eligibility traces vector
        int i_prev_action;        // The action a decided for s
        int i_curr_action;        // The action a' decided for s'
        FloatVect v_Q;            // Q[a] value of action a in state s 
        FeatureMap* pv_curr_features_map;  // a map from action number to a 
                                  // vector of active feature indecies for the 
                                  // current time step (s')
        IntVect* pv_num_nonzero_in_f;  // Number of non-zero values in the 
                                  // featur-vector, corresponding to each action
        int i_episode_counter;    // Counts the number of episodes we have seen
		int i_frame_counter;	  // Counts the number of frames we have seen
		int i_shrink_weights_frq; // How often to remove the smallest value in 
								  // the weights vactor 
		string s_shrink_weights_method;	// [sqr_root_num_frames/linear_shrink/
										//  const_shrink]
								  // Number of values that will be kept in the
								  // weights vector is: 
								  //  . sqr_root(i_frame_counter) * 
								  //	shrink_weights_const
								  // or
								  //  . shrink_weights_const * 
								  //	number_of_current_non_zero_values
								  // or
								  //  . number_of_current_non_zero_values - 
								  //    shrink_weights_const
		float f_shrink_weights_const; // ee documents for shrink_weights_method
        int i_save_weights_freq;  // How often (based on number of episodes)
                                  //  should the weights be saved to disk.
                                  //  0 means that weigths will not be saved        
        int i_epsilon_dim_start;  // Starts diminishing epsilon 
                                  // after this many episodes
		float f_eps_dim_delta;	  // The amount to subtract from delta per epis
        int i_max_nonzero_traces; // Max number of non-zero values in pv_traces
        int i_num_nonzero_traces; // Number of non-zero value in v_traces
        double f_minimum_trace;    // Minimum non-zero value in v_traces
        IntVect* pv_nonzero_traces_ind;//Indecies of non-zero values in v_traces
        IntVect* pv_nonzero_traces_inv_ind; // inverse of pv_nonzero_traces_ind
        
        
        
};

class RLSarsaLambdaWithIDBD : public RLSarsaLambda {
	/* *************************************************************************
	 *	Sarsa-Lambda with Incremental Delta-Bar-Delta
	 *  (This version doesn't need a alpha parameter for stepsize. Instead it
	 *  dynamically calculates a step-size for every bit of the feature vector
	 *  See "Adaptive Bias by Gradient Descent: An Incremental Version of 
	 *  Delta-Bar-Delta" by Rich Sutton, 1992
	 ************************************************************************ */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        RLSarsaLambdaWithIDBD(	OSystem* _osystem, int feature_vec_size, 
								int num_actions,  
								int save_weights_freq) ;
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~RLSarsaLambdaWithIDBD();
		
		/* *********************************************************************
            Update weights with nonzero traces using td-error
         * ****************************************************************** */
        virtual void updateWeights(double delta);
		
		double f_theta;		// the new meta-step-size
		FloatArr* pv_h;     // the h vector (See the otiginal paper)
		FloatArr* pv_beta;  // the beta vector (See the otiginal paper)
};
		
class RLSarsaLambdaWithDeltaBarDelta : public RLSarsaLambda {
	/* *************************************************************************
	 *	Sarsa-Lambda with Delta-Bar-Delta (this is not Rich's iDBD, but the 
	 *	version previous to it)
	 *  
	 *  See "Increased rates of Convergence Through Learning rate Adaptation" by 
	 *	Robert Jacobs, 1988
	 ************************************************************************ */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        RLSarsaLambdaWithDeltaBarDelta(	OSystem* _osystem, int feature_vec_size, 
								int num_actions, double eps_start, 
								double k, double phi, double theta,
								int save_weights_freq) ;
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~RLSarsaLambdaWithDeltaBarDelta();
		
		/* *********************************************************************
            Update weights with nonzero traces using td-error
         * ****************************************************************** */
        virtual void updateWeights(double delta);
		
		double f_k, f_phi, f_theta;	// meta-step-size parameters
		FloatArr* pv_eps;   // epsilon h vector (See the otiginal paper)
		FloatArr* pv_delta_bar;  // the delta_bar vector (See the otiginal paper)
};
		


#endif
