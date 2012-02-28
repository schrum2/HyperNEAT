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

// Search a map for a key and returns default value if not found
template <typename K, typename V>
  V GetWithDef(const std::map <K,V> & m, const K & key, const V & defval ) {
  typename std::map<K,V>::const_iterator it = m.find( key );
  if ( it == m.end() ) {
    return defval;
  }
  else {
    return it->second;
  }
};

// Calculates the information entropy of a random variable whose values and
// frequencies are provided by the map m.
template <typename K>
float compute_entropy(const std::map <K,int> & m, int count_sum) {
  float velocity_entropy = 0;
  typename std::map<K,int>::const_iterator it;
  for (it=m.begin(); it!=m.end(); ++it) {
    int count = it->second;
    float p_x = count / (float) count_sum;
    velocity_entropy -= p_x * log(p_x);
  }
  return velocity_entropy;
};

class SelfDetectionAgent : public PlayerAgent { 
 public:
  SelfDetectionAgent(GameSettings* _game_settings, OSystem* _osystem);
        
  // Returns a random action from the set of possible actions
  virtual Action agent_step(const IntMatrix* screen_matrix, 
                            const IntVect* console_ram, 
                            int frame_number);

  // Looks through objects attempting to find one that we are controlling
  void identify_self();

  void display_screen(const IntMatrix& screen_matrix);

  void printVelHistory(CompositeObject& obj);

  VisualProcessor visProc;
  int screen_width, screen_height;
  int max_history_len;

  long self_id; // ID of the object which corresponds to the "self"
};

#endif




