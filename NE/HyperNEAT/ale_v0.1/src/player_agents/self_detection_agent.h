/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 * *****************************************************************************/

#ifndef SELF_DETECTION_AGENT_H
#define SELF_DETECTION_AGENT_H

#include <deque>
#include "common_constants.h"
#include <boost/unordered_set.hpp>
#include "player_agent.h"
#include <set>
#include "visual_processor.h"

/** The self detection agent is tasked with selecting actions to make clear
    which object on screen is the entity being controlled.
**/
class SelfDetectionAgent : public PlayerAgent { 
 public:
  SelfDetectionAgent(GameSettings* _game_settings, OSystem* _osystem);
        
  // Returns a random action from the set of possible actions
  virtual Action agent_step(const IntMatrix* screen_matrix, 
                            const IntVect* console_ram, 
                            int frame_number);

  void display_screen(const IntMatrix& screen_matrix);

  VisualProcessor visProc;
  int screen_width, screen_height;
  int max_history_len;

  long self_id; // ID of the object which corresponds to the "self"
};

#endif




