/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  common_constants.h
 *
 *  Defines a set of constants used by various parts of the player agent code
 *
 **************************************************************************** */
#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include <cassert>
#include <vector>
#include <valarray>
#include <cstdlib> 
#include "bspf.hxx"


// Define actions
enum Action {
    PLAYER_A_NOOP           = 0, 
    PLAYER_A_FIRE           = 1, 
    PLAYER_A_UP             = 2,
    PLAYER_A_RIGHT          = 3,
    PLAYER_A_LEFT           = 4,
    PLAYER_A_DOWN           = 5,
    PLAYER_A_UPRIGHT        = 6,
    PLAYER_A_UPLEFT         = 7,
    PLAYER_A_DOWNRIGHT      = 8,
    PLAYER_A_DOWNLEFT       = 9,
    PLAYER_A_UPFIRE         = 10,
    PLAYER_A_RIGHTFIRE      = 11,
    PLAYER_A_LEFTFIRE       = 12,
    PLAYER_A_DOWNFIRE       = 13,
    PLAYER_A_UPRIGHTFIRE    = 14,
    PLAYER_A_UPLEFTFIRE     = 15,
    PLAYER_A_DOWNRIGHTFIRE  = 16,
    PLAYER_A_DOWNLEFTFIRE   = 17, 
    PLAYER_B_NOOP           = 18,
    PLAYER_B_FIRE           = 19,
    PLAYER_B_UP             = 20,
    PLAYER_B_RIGHT          = 21,
    PLAYER_B_LEFT           = 22,
    PLAYER_B_DOWN           = 23,
    PLAYER_B_UPRIGHT        = 24,
    PLAYER_B_UPLEFT         = 25,
    PLAYER_B_DOWNRIGHT      = 26,
    PLAYER_B_DOWNLEFT       = 27,
    PLAYER_B_UPFIRE         = 28,
    PLAYER_B_RIGHTFIRE      = 29,
    PLAYER_B_LEFTFIRE       = 30,
    PLAYER_B_DOWNFIRE       = 31,
    PLAYER_B_UPRIGHTFIRE    = 32,
    PLAYER_B_UPLEFTFIRE     = 33,
    PLAYER_B_DOWNRIGHTFIRE  = 34,
    PLAYER_B_DOWNLEFTFIRE   = 35,
    RESET		    = 40,
    UNDEFINED               = 41,
    RANDOM		    = 42,
    LAST_ACTION_INDEX	    = 50
};

enum e_colors {
  BLACK = 0,
  BLUE  = 1,
  RED   = 2,
  PINK  = 3,
  GREEN = 4,
  CYAN  = 5,
  YELLOW= 6,
  WHITE = 7
};

std::string action_to_string(Action a);

enum EpisodeStatus {
    INITIAL_DELAY,          // The delay at the very beginning of the game
    RESTART_DELAY,          // Delay after restarting the game
    START_EPISODE,          // We are about to start the episode
    ACTION_EXPLOR,          // Exploration by trying individual actions
    AGENT_ACTING,           // we are in the middle of an episode
    HUMAN_CONTROLLING,      // The end user is controlling the game
};


//  Define datatypes
typedef valarray<int>       IntArr;
typedef valarray<bool>      BitArr;
typedef valarray<double>    FloatArr;
typedef vector<int>         IntVect;
typedef vector<Action>      ActionVect;
typedef vector<double>      FloatVect;
typedef vector<IntVect>     IntMatrix;
typedef vector<FloatVect>   FloatMatrix;
typedef vector<IntArr>      FeatureMap; // Map from action number to feature vec
class RegionObject;
typedef vector < RegionObject* > RegionObjectList;

// Other constant values
#define RAM_LENGTH 128
#define MAX_NUM_TILING_VARS 10  // Maximum dimension for tiling (i.e. the          
                                // maximum number of floats we send to tiles2 
#define CUSTOM_PALETTE_SIZE 1020	// Number of colors in custom palette
#define BLACK_COLOR_IND 1000		// color index in custom palette for Black 
#define RED_COLOR_IND 1001			// color index in custom palette for Red 
#define WHITE_COLOR_IND 1003		// color index in custom palette for White 
#define SECAM_COLOR_IND 1010		// starting index in the custom palette for
									// the eight SECAM colors


#endif

