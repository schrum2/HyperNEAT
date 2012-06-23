/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  single_action_agent.h
 *
 * The implementation of the SingleActionAgent class, which always return a single 
 * action
 **************************************************************************** */
 
#ifndef SINGLE_ACTION_AGENT_H
#define SINGLE_ACTION_AGENT_H

#include "common_constants.h"
#include "player_agent.h"

class SingleActionAgent : public PlayerAgent {

    public:
        SingleActionAgent(GameSettings* _game_settings, OSystem* _osystem);
        
        /* *********************************************************************
            Returns a random action from the set of possible actions
         ******************************************************************** */
        virtual Action agent_step(  const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_number);

		Action e_action;	// The action we will always return
};

#endif
