/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  freeway_agent.h
 *
 *  The implementation of the FreewayAgent class, a subclass of ClassAgent,
 *  which is specifically desinged to solve the game Freeway 
 **************************************************************************** */

#ifndef FREEWAY_AGENT_H
#define FREEWAY_AGENT_H

#include <vector>
#include "common_constants.h"
#include "class_agent.h"


class FreewayAgent : public ClassAgent {
    /* *************************************************************************
        A specifically designed agent to play Freeway. Used to run freeway 
		specific experiments.
		
		- pm_debug_best_act		A 2D matrix used for debugging plots
		- pm_debug_best_val		A 2D matrix used for debugging plots
    ************************************************************************* */

    public:
        FreewayAgent(GameSettings* _game_settings, OSystem* _osystem);
        virtual ~FreewayAgent();
        
        /* *********************************************************************
            Returns an action from the set of possible actions.
            Runs one step of the Sarsa-Lambda algorithm
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
			Calculates the size of the full feature-vector
		 ******************************************************************** */
		virtual void calc_feature_vec_size(void);
	
        /* *********************************************************************
            Generates a feature vector based extracted location of the chicken and the cars
		 ******************************************************************** */
        virtual void generate_feature_vec(void);

		/* *********************************************************************
			Used for Debugging Freeway.
			Plots the best value based on our current w, for different values
			of the chicken position.
		 ******************************************************************** */
		void plot_best_value_per_state(void);
        
		
		/* *********************************************************************
			Manually extracts the current y-value of the chicken
		 ******************************************************************** */
		 int get_chicken_y(const IntMatrix* pv_screen);
		 
		 /* *********************************************************************
			Used for Debugging Freeway.
			Manually extracts the current x-values of the cars
		 ******************************************************************** */
		 void get_cars_x(const IntMatrix* pv_screen);
		
		FloatMatrix* pm_debug_best_val;
		IntVect v_debug_car_y_vals;
		IntVect v_debug_car_x_vals;
		IntVect v_debug_car_color_ind;
		FloatVect v_debug_car_vel;
};


#endif
