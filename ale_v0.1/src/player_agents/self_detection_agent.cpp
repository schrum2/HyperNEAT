/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 **************************************************************************** */

#include "self_detection_agent.h"
#include "../common/random_tools.h"
#include "../emucore/m6502/src/System.hxx"
#include <limits>
#include <sstream>
#include <omp.h>

SelfDetectionAgent::SelfDetectionAgent(GameSettings* _game_settings, OSystem* _osystem) : 
    PlayerAgent(_game_settings, _osystem), visProc(_osystem, _game_settings)
{
  // Get the height and width of the screen
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  screen_width  = mediasrc.width();
  screen_height = mediasrc.height();
};

// Returns a random action from the set of possible actions
Action SelfDetectionAgent::agent_step(const IntMatrix* screen_matrix, 
                                      const IntVect* console_ram, 
                                      int frame_number) {
  Action action = PlayerAgent::agent_step(screen_matrix, console_ram, frame_number);
  if (action == UNDEFINED)
     action = choice <Action>(p_game_settings->pv_possible_actions);

  visProc.process_image(screen_matrix,action);

  return action;
}

// Overrides the normal display screen method to alter our display
void SelfDetectionAgent::display_screen(const IntMatrix& screen_matrix) {
  IntMatrix screen_cpy(screen_matrix);
  visProc.display_screen(screen_cpy);
  PlayerAgent::display_screen(screen_cpy);
};

