/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  random_agent.h
 *
 * The implementation of the RandomAgent class, which acts randomly in the 
 * environment
 **************************************************************************** */

#include "random_agent.h"
#include "../common/random_tools.h"
#include "../emucore/Serializer.hxx"
#include "../emucore/Deserializer.hxx"
#include "../emucore/m6502/src/System.hxx"
#include <sstream>

RandomAgent::RandomAgent(GameSettings* _game_settings, OSystem* _osystem) : 
    PlayerAgent(_game_settings, _osystem) {
}

/* *********************************************************************
    Returns a random action from the set of possible actions
 ******************************************************************** */
Action RandomAgent::agent_step( const IntMatrix* screen_matrix, 
                                const IntVect* console_ram, 
								int frame_number) {
    Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram,
													frame_number);
    if (special_action != UNDEFINED) {
        return special_action;  // We are resettign or in a delay 
    }
	

    return choice <Action> (p_game_settings->pv_possible_actions);
}
