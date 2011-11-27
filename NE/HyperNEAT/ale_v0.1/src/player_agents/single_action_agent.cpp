/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  single_action_agent.cpp
 *
 * The implementation of the SingleAction class, which always return a single 
 * action
 **************************************************************************** */

#include "single_action_agent.h"


SingleActionAgent::SingleActionAgent(GameSettings* _game_settings, 
									 OSystem* _osystem) : 
    PlayerAgent(_game_settings, _osystem) {
	int action_index = p_osystem->settings().getInt("single_action_index", true);
	if (action_index < 0 || action_index >= pv_possible_actions->size()) {
		cout<<"Invalid value for single_action_index: " << action_index << endl;
		cerr<<"Invalid value for single_action_index: " << action_index << endl;
		exit(-1);
	}
	e_action = (*p_game_settings->pv_possible_actions)[action_index];
	cout << "Only applying action: " << action_to_string(e_action) << endl;
}

/* *********************************************************************
    Returns a random action from the set of possible actions
 ******************************************************************** */
Action SingleActionAgent::agent_step(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_number) {
    Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram,
													frame_number);
    if (special_action != UNDEFINED) {
        return special_action;  // We are resettign or in a delay 
    }
    return e_action;
}
