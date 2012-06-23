/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  game_controller.h
 *
 *  The implementation of the GameController class, which is the superclass for
 *  other controller classes. A controller class sists between stella, and 
 *  whatever mechansim we are using to control stella, i.e. FIFA pipes and
 *  external code, or internal player_agent classes.
 **************************************************************************** */
#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "../emucore/OSystem.hxx"
#include "../emucore/m6502/src/System.hxx"
#include "../player_agents/common_constants.h"
#include "../common/export_screen.h"

#define PADDLE_DELTA 23000
#define PADDLE_MIN 250000 
#define PADDLE_MAX 800000
// #define PADDLE_MAX 1000000

class GameController {
    /* *************************************************************************
        This is the superclass for all controller classes. 
        A controller class sists between stella, and whatever mechansim we are 
        using to control stella, i.e. FIFA pipes and external code, or 
        internal player_agent classes.
        
        Instance Variables:
            p_osystem                 Pointer to the stella's OSystem object
            p_global_event_obj        Pointer to the global event object
            i_screen_width            Width of the screen
            i_screen_height           Height of the screen
            b_send_screen_matrix;     When True, we will send the screen matrix 
            b_send_console_ram;		  When True, we will send the console ram 
            i_skip_frames_num;		  We skip this number of frames after 
                                      sending a frame
            i_skip_frames_counter;    Counts how many frames we have skipped
            e_previous_a_action;      Action applied for player A/B during the 
            e_previous_b_action;      last farme (used when skipping frames)
			b_left_player_paddles	  When true, left player uses paddles
			b_rightt_player_paddles	  When true, right player uses paddles
			i_left_paddle_curr_x	  Current x value for the left-paddle
			i_right_paddle_curr_x	  Current x value for the right-paddle

    ************************************************************************* */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        GameController(OSystem* _osystem);
        
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~GameController();
        
        /* *********************************************************************
            This is called on every iteration of the main loop. It is resposible 
            passing the framebuffer and the RAM content to whatever AI module we 
            are using, and applying the returned actions.
         * ****************************************************************** */
        virtual void update() = 0;

        /* *********************************************************************
            Applies the actions recieved from the controller for player A and B
         * ********************************************************************/
        static void apply_action(	Event* p_global_event_obj, 
									int player_a_action, int player_b_action); 

        /* *********************************************************************
            Unpresses all control relevant keys
         * ********************************************************************/
        static void reset_keys(Event* p_global_event_obj);

        /* *********************************************************************
         *  Reads a byte from console ram
         *	
         *	The code is mainly based on RamDebug.cxx
         * ********************************************************************/
        int read_ram(int offset); 
        
        


    protected:
		/* *********************************************************************
         *  Calculates the Paddle resistance, based on the given x val
         * ********************************************************************/
		int calc_paddle_resistance(int x_val);
		
		/* *********************************************************************
         *  Updates the positions of the paddles, and sets an event for 
		 *  updating the corresponding paddle's resistance
         * ********************************************************************/
		 void update_paddles_positions(int delta_left, int delta_right);
		
        OSystem* p_osystem;         // Pointer to the stella's OSystem object
        Event* p_global_event_obj;  // Pointer to the global event object
        
        int i_screen_width;         // Width of the screen
        int i_screen_height;        // Height of the screen
        uInt8* pi_curr_frame_buffer;// Pointer to the current framebuffer (used
                                    // to read the screen matrix)
        Console* p_console;         // Pointer to the Console object
        System* p_emulator_system;  // Pointer to the emulator system  (used to
                                    // read the system RAM)
        bool b_send_screen_matrix;  // When True, we will send the screen matrix 
        bool b_send_console_ram;    // When True, we will send the console ram 
        int i_skip_frames_num;      // We skip this number of frames after 
                                    // sending a frame
        int i_skip_frames_counter;  // Counts how many frames we have skipped
        Action e_previous_a_action; // Action applied for player A/B during the 
        Action e_previous_b_action; // last farme (used when skipping frames)
		bool b_left_player_paddles;	// When true, left player uses paddles
		bool b_right_player_paddles;//When true, right player uses paddles
		int i_left_paddle_curr_x;	// Current x value for the left-paddle
		int i_right_paddle_curr_x;	// Current x value for the left-paddle
		
};


#endif
