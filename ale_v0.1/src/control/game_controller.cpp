/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  game_controller.cpp
 *
 *  The implementation of the GameController class, which is the superclass for
 *  other controller classes. A controller class sists between stella, and 
 *  whatever mechansim we are using to control stella, i.e. FIFA pipes and
 *  external code, or internal player_agent classes.
 **************************************************************************** */

#include "game_controller.h"


/* *********************************************************************
    Constructor
 ******************************************************************** */
GameController::GameController(OSystem* _osystem) {
    p_osystem = _osystem;
    p_global_event_obj = p_osystem->event();
    p_console = &(p_osystem->console());
    MediaSource& mediasrc = p_console->mediaSource();
    pi_curr_frame_buffer = mediasrc.currentFrameBuffer();
    p_emulator_system = &(p_console->system());
    i_screen_width  = mediasrc.width();
    i_screen_height = mediasrc.height();
    
	
    b_send_screen_matrix    = true;
    b_send_console_ram      = true;
    i_skip_frames_num       = 0;
    i_skip_frames_counter   = 0;
    e_previous_a_action     = PLAYER_A_NOOP;
    e_previous_b_action     = PLAYER_B_NOOP;
	i_left_paddle_curr_x	= ((PADDLE_MAX - PADDLE_MIN) / 2) + PADDLE_MIN;
	i_right_paddle_curr_x	= ((PADDLE_MAX - PADDLE_MIN) / 2) + PADDLE_MIN;
	if (p_osystem->console().properties().get(Controller_Left) == "PADDLES") {
		cout << "Left player using Paddles" << endl;
		b_left_player_paddles = true;
		int left_resistance = calc_paddle_resistance(i_left_paddle_curr_x);	
		p_global_event_obj->set(Event::PaddleZeroResistance, left_resistance);
	} else {
		b_left_player_paddles = false;
	}
	if (p_osystem->console().properties().get(Controller_Right) == "PADDLES") {
		cout << "Right player using Paddles" << endl;
		b_right_player_paddles = true;	
		int right_resistance = calc_paddle_resistance(i_right_paddle_curr_x);	
		p_global_event_obj->set(Event::PaddleOneResistance, right_resistance);
	} else {
		b_right_player_paddles = false;
	}

}
        
/* *********************************************************************
    Deconstructor
 ******************************************************************** */
GameController::~GameController() {
}


/* ***************************************************************************
 *  Function apply_action
 *  Applies the actions recieved from the controller for player A and B
 * ***************************************************************************/
void GameController::apply_action(	Event* p_global_event_obj, 
									int player_a_action, int player_b_action) {
	reset_keys(p_global_event_obj);
	switch(player_a_action)	{
		case PLAYER_A_NOOP: 
			break; 
			
		case PLAYER_A_FIRE: 
			p_global_event_obj->set(Event::JoystickZeroFire, 1);
			break; 
			
		case PLAYER_A_UP: 
			p_global_event_obj->set(Event::JoystickZeroUp, 1);
			break; 
			
		case PLAYER_A_RIGHT: 
			p_global_event_obj->set(Event::JoystickZeroRight, 1);
			break; 
			
		case PLAYER_A_LEFT: 
			p_global_event_obj->set(Event::JoystickZeroLeft, 1);
			break; 
			
		case PLAYER_A_DOWN: 
			p_global_event_obj->set(Event::JoystickZeroDown, 1);
			break; 
			
		case PLAYER_A_UPRIGHT: 
			p_global_event_obj->set(Event::JoystickZeroUp, 1);
			p_global_event_obj->set(Event::JoystickZeroRight, 1);
			break; 
			
		case PLAYER_A_UPLEFT: 
			p_global_event_obj->set(Event::JoystickZeroUp, 1);
			p_global_event_obj->set(Event::JoystickZeroLeft, 1);
			break; 
			
		case PLAYER_A_DOWNRIGHT: 
			p_global_event_obj->set(Event::JoystickZeroDown, 1);
			p_global_event_obj->set(Event::JoystickZeroRight, 1);
			break; 
			
		case PLAYER_A_DOWNLEFT: 
			p_global_event_obj->set(Event::JoystickZeroDown, 1);
			p_global_event_obj->set(Event::JoystickZeroLeft, 1);
			break; 
			
		case PLAYER_A_UPFIRE: 
			p_global_event_obj->set(Event::JoystickZeroUp, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1);
			break; 
			
		case PLAYER_A_RIGHTFIRE: 
			p_global_event_obj->set(Event::JoystickZeroRight, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1); 
			break; 
			
		case PLAYER_A_LEFTFIRE: 
			p_global_event_obj->set(Event::JoystickZeroLeft, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1); 
			break; 
			
		case PLAYER_A_DOWNFIRE: 
			p_global_event_obj->set(Event::JoystickZeroDown, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1);
			break; 
			
		case PLAYER_A_UPRIGHTFIRE: 
			p_global_event_obj->set(Event::JoystickZeroUp, 1);
			p_global_event_obj->set(Event::JoystickZeroRight, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1);
			break; 
			
		case PLAYER_A_UPLEFTFIRE: 
			p_global_event_obj->set(Event::JoystickZeroUp, 1);
			p_global_event_obj->set(Event::JoystickZeroLeft, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1); 
			break; 
			
		case PLAYER_A_DOWNRIGHTFIRE: 
			p_global_event_obj->set(Event::JoystickZeroDown, 1);
			p_global_event_obj->set(Event::JoystickZeroRight, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1); 
			break; 
			
		case PLAYER_A_DOWNLEFTFIRE: 
			p_global_event_obj->set(Event::JoystickZeroDown, 1);
			p_global_event_obj->set(Event::JoystickZeroLeft, 1);
			p_global_event_obj->set(Event::JoystickZeroFire, 1);
			break; 
		case RESET:
			p_global_event_obj->set(Event::ConsoleReset, 1);
			cerr << "Sending Reset..." << endl;
			break;
		default: 
			cerr << "Invalid Player A Action: " << player_a_action;
			exit(-1); 
		
	}

	switch(player_b_action)	{
	case PLAYER_B_NOOP: 
			break; 
			
		case PLAYER_B_FIRE: 
			p_global_event_obj->set(Event::JoystickOneFire, 1);
			break; 
			
		case PLAYER_B_UP: 
			p_global_event_obj->set(Event::JoystickOneUp, 1);
			break; 
			
		case PLAYER_B_RIGHT: 
			p_global_event_obj->set(Event::JoystickOneRight, 1);
			break; 
			
		case PLAYER_B_LEFT: 
			p_global_event_obj->set(Event::JoystickOneLeft, 1);
			break; 
			
		case PLAYER_B_DOWN: 
			p_global_event_obj->set(Event::JoystickOneDown, 1);
			break; 
			
		case PLAYER_B_UPRIGHT: 
			p_global_event_obj->set(Event::JoystickOneUp, 1);
			p_global_event_obj->set(Event::JoystickOneRight, 1);
			break; 
			
		case PLAYER_B_UPLEFT: 
			p_global_event_obj->set(Event::JoystickOneUp, 1);
			p_global_event_obj->set(Event::JoystickOneLeft, 1);
			break; 
			
		case PLAYER_B_DOWNRIGHT: 
			p_global_event_obj->set(Event::JoystickOneDown, 1);
			p_global_event_obj->set(Event::JoystickOneRight, 1);
			break; 
			
		case PLAYER_B_DOWNLEFT: 
			p_global_event_obj->set(Event::JoystickOneDown, 1);
			p_global_event_obj->set(Event::JoystickOneLeft, 1);
			break; 
			
		case PLAYER_B_UPFIRE: 
			p_global_event_obj->set(Event::JoystickOneUp, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1);
			break; 
			
		case PLAYER_B_RIGHTFIRE: 
			p_global_event_obj->set(Event::JoystickOneRight, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1); 
			break; 
			
		case PLAYER_B_LEFTFIRE: 
			p_global_event_obj->set(Event::JoystickOneLeft, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1); 
			break; 
			
		case PLAYER_B_DOWNFIRE: 
			p_global_event_obj->set(Event::JoystickOneDown, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1);
			break; 
			
		case PLAYER_B_UPRIGHTFIRE: 
			p_global_event_obj->set(Event::JoystickOneUp, 1);
			p_global_event_obj->set(Event::JoystickOneRight, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1);
			break; 
			
		case PLAYER_B_UPLEFTFIRE: 
			p_global_event_obj->set(Event::JoystickOneUp, 1);
			p_global_event_obj->set(Event::JoystickOneLeft, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1); 
			break; 
			
		case PLAYER_B_DOWNRIGHTFIRE: 
			p_global_event_obj->set(Event::JoystickOneDown, 1);
			p_global_event_obj->set(Event::JoystickOneRight, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1); 
			break; 
			
		case PLAYER_B_DOWNLEFTFIRE: 
			p_global_event_obj->set(Event::JoystickOneDown, 1);
			p_global_event_obj->set(Event::JoystickOneLeft, 1);
			p_global_event_obj->set(Event::JoystickOneFire, 1);
			break; 
		case RESET:
			p_global_event_obj->set(Event::ConsoleReset, 1);
			cerr << "Sending Reset..." << endl;
			break;
		default: 
			cerr << "Invalid Player B Action: " << player_b_action << endl;
			exit(-1); 
	}
}

/* ***************************************************************************
    Function reset_keys
    Unpresses all control-relavant keys
 * ***************************************************************************/
void GameController::reset_keys(Event* p_global_event_obj) {
//TODO: FIX	p_global_event_obj->set(Event::PaddleZeroFire, 0);
	p_global_event_obj->set(Event::ConsoleReset, 0);
	p_global_event_obj->set(Event::JoystickZeroFire, 0);
	p_global_event_obj->set(Event::JoystickZeroUp, 0);
	p_global_event_obj->set(Event::JoystickZeroDown, 0);
	p_global_event_obj->set(Event::JoystickZeroRight, 0);
	p_global_event_obj->set(Event::JoystickZeroLeft, 0);
	p_global_event_obj->set(Event::JoystickOneFire, 0);
	p_global_event_obj->set(Event::JoystickOneUp, 0);
	p_global_event_obj->set(Event::JoystickOneDown, 0);
	p_global_event_obj->set(Event::JoystickOneRight, 0);
	p_global_event_obj->set(Event::JoystickOneLeft, 0);

}

/* ***************************************************************************
 *  Function read_ram
 *  Reads a byte from console ram
 *	
 *	The code is mainly based on RamDebug.cxx
 * ***************************************************************************/
int GameController::read_ram(int offset) {
	offset &= 0x7f; // there are only 128 bytes
	return p_emulator_system->peek(offset + 0x80);
}

/* ***************************************************************************
 *  Calculates the Paddle resistance, based on the given x val
 * ***************************************************************************/
int GameController::calc_paddle_resistance(int x_val) {
	return x_val;	// this is different from the original stella implemebtation
}

/* *********************************************************************
 *  Updates the positions of the paddles, and sets an event for 
 *  updating the corresponding paddle's resistance
 * ********************************************************************/
 void GameController::update_paddles_positions(int delta_left, int delta_right){
	if (delta_left != 0) {
		i_left_paddle_curr_x += delta_left;
		if (i_left_paddle_curr_x < PADDLE_MIN) {
			i_left_paddle_curr_x = PADDLE_MIN;
		} 
		if (i_left_paddle_curr_x >  PADDLE_MAX) {
			i_left_paddle_curr_x = PADDLE_MAX;
		}
		int left_resistance = calc_paddle_resistance(i_left_paddle_curr_x);
		p_global_event_obj->set(Event::PaddleZeroResistance, left_resistance);
	}
	if (delta_right != 0) {
		i_right_paddle_curr_x += delta_right;
		if (i_right_paddle_curr_x < PADDLE_MIN) {
			i_right_paddle_curr_x = PADDLE_MIN;
		} 
		if (i_right_paddle_curr_x >  PADDLE_MAX) {
			i_right_paddle_curr_x = PADDLE_MAX;
		}
		int right_resistance = calc_paddle_resistance(i_right_paddle_curr_x);
		p_global_event_obj->set(Event::PaddleOneResistance, right_resistance);
	}
}
