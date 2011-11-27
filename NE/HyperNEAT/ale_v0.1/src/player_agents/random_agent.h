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

#ifndef RANODM_AGENT_H
#define RANDOM_AGENT_H

#include "common_constants.h"
#include "player_agent.h"

class RandomAgent : public PlayerAgent {

    public:
        RandomAgent(GameSettings* _game_settings, OSystem* _osystem);
        
        /* *********************************************************************
            Returns a random action from the set of possible actions
         ******************************************************************** */
        virtual Action agent_step(  const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_number);

        
};

#endif




