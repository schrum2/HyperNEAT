/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  game_settings.cpp
 *  stella_agent
 *
 *  The implementation of the GameSettings class, which is the superclass for
 *  other game-settings classes. These classes contain game-specific information
 *  and functionality. 
 **************************************************************************** */

#include "game_settings.h"


/* ****************************************************************************
    Class GameSettings
    
    This is the superclass for other game-settings classes.
    These classes contain game-specific data and functionality, such
    as how to determine the reward from the game, or how to tell
**************************************************************************** */

/* **********************************************************************
    Constructor
 ********************************************************************** */
GameSettings::GameSettings( string _rom_file,  
                            bool _uses_screen_matrix, bool _uses_console_ram,
                            int _skip_frames_num, int _delay_after_restart, 
                            Action _first_action) : 
    s_rom_file(_rom_file), 
    b_uses_screen_matrix(_uses_screen_matrix),
    b_uses_console_ram(_uses_console_ram),
    i_skip_frames_num(_skip_frames_num),
    i_delay_after_restart(_delay_after_restart), 
    e_first_action(_first_action)     {
    pv_possible_actions = new ActionVect;
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_UPRIGHT);
    pv_possible_actions->push_back(PLAYER_A_UPLEFT);
    pv_possible_actions->push_back(PLAYER_A_DOWNRIGHT);
    pv_possible_actions->push_back(PLAYER_A_DOWNLEFT);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPRIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPLEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNRIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNLEFTFIRE);
    i_prev_score = 0;
	i_frames_since_last_restart = 0;
}

/* *********************************************************************
    Deconstructor
 ******************************************************************** */
GameSettings::~GameSettings() {
    if (pv_possible_actions) {
        delete pv_possible_actions;
    }
}
    
/* *********************************************************************
    Generates an instance of one of the GameSettings subclasses, based 
    on the name of the current ROM file.
    Note 1: If you add a new GameSettngs subclass, you need to also 
            update it here
    Note 2: The caller is resposible for deleting the returned pointer
******************************************************************** */
GameSettings* GameSettings::generate_game_Settings_instance(string rom_file) {
    GameSettings* new_game_settings = NULL;
	size_t slash_ind = rom_file.find_last_of("/\\");
	rom_file = rom_file.substr(slash_ind + 1);

    if (rom_file == "freeway.bin") {
        new_game_settings = new FreewaySettings();
    } else if (rom_file == "space_invaders.bin") {
        new_game_settings = new SpaceInvadersSettings();
    } else if (rom_file == "asterix.bin") {
        new_game_settings = new AstrixSettings();
    } else if (rom_file == "seaquest.bin") {
        new_game_settings = new SeaquestSettings();
    } else if (rom_file == "tennis.bin") {
        new_game_settings = new TenniSettings();
    } else if (rom_file == "wizard_of_wor.bin") {
        new_game_settings = new WizardofWorSettings();
    } else if (rom_file == "krull.bin") {
        new_game_settings = new KrullSettings();
    } else if (rom_file == "atlantis.bin") {
        new_game_settings = new AtlantisSettings();
    } else if (rom_file == "hero.bin") {
        new_game_settings = new HEROSettings();
    } else if (rom_file == "riverraid.bin") {
        new_game_settings = new RiverRaidSettings();
    } else if (rom_file == "venture.bin") {
        new_game_settings = new VentureSettings();
    } else if (rom_file == "crazy_climber.bin") {
        new_game_settings = new CrazyClimberSettings();
    } else if (rom_file == "chopper_command.bin") {
        new_game_settings = new ChopperCommandSettings();
    } else if (rom_file == "ice_hockey.bin") {
        new_game_settings = new IceHockeySettings();
    } else if (rom_file == "montezuma_revenge.bin") {
        new_game_settings = new MontezumaRevengeSettings();
    } else if (rom_file == "gravitar.bin") {
        new_game_settings = new GravitarSettings();
    } else if (rom_file == "carnival.bin") {
        new_game_settings = new CarnivalSettings();
    } else if (rom_file == "beam_rider.bin") {
        new_game_settings = new BeamRiderSettings();
    } else if (rom_file == "boxing.bin") {
        new_game_settings = new BoxingSettings();
    } else if (rom_file == "berzerk.bin") {
        new_game_settings = new BerzerkSettings();
    } else if (rom_file == "star_gunner.bin") {
        new_game_settings = new StarGunnerSettings();
    } else if (rom_file == "amidar.bin") {
        new_game_settings = new AmidarSettings();
    } else if (rom_file == "video_pinball.bin") {
        new_game_settings = new VideoPinballSettings();
    } else if (rom_file == "gopher.bin") {
        new_game_settings = new GopherSettings();
    } else if (rom_file == "time_pilot.bin") {
        new_game_settings = new TimePilotSettings();
    } else if (rom_file == "fishing_derby.bin") {
        new_game_settings = new FishingDerbySettings();
    } else if (rom_file == "battle_zone.bin") {
        new_game_settings = new BattleZoneSettings();
    } else if (rom_file == "pitfall.bin") {
        new_game_settings = new PitfallSettings();
    } else if (rom_file == "robotank.bin") {
        new_game_settings = new RoboTankSettings();
    } else if (rom_file == "jamesbond.bin") {
        new_game_settings = new JamesBondSettings();
    } else if (rom_file == "road_runner.bin") {
        new_game_settings = new RoadRunnerSettings();
    } else if (rom_file == "astroids.bin") {
        new_game_settings = new AstroidsSettings();
    } else if (rom_file == "tutankham.bin") {
        new_game_settings = new TutankhamSettings();
    } else if (rom_file == "enduro.bin") {
        new_game_settings = new EnduroSettings();
    } else if (rom_file == "pooyan.bin") {
        new_game_settings = new PooyanSettings();
    } else if (rom_file == "air_raid.bin") {
        new_game_settings = new AirRaidSettings();
    } else if (rom_file == "alien.bin") {
        new_game_settings = new AlienSettings();
    } else if (rom_file == "centipede.bin") {
        new_game_settings = new CentipedeSettings();
    } else if (rom_file == "solaris.bin") {
        new_game_settings = new SolarisSettings();
    } else if (rom_file == "bank_heist.bin") {
        new_game_settings = new BankHeistSettings();
    } else if (rom_file == "zaxxon.bin") {
        new_game_settings = new ZaxxonSettings();
    } else if (rom_file == "ms_pacman.bin") {
        new_game_settings = new MsPacman();
    } else if (rom_file == "skiing.bin") {
        new_game_settings = new SkiingSettings();
    } else if (rom_file == "double_dunk.bin") {
        new_game_settings = new DoubleDunkSettings();
    } else if (rom_file == "joust.bin") {
        new_game_settings = new JoustSettings();
    } else if (rom_file == "kung_fu_master.bin") {
        new_game_settings = new KungFuMasterSettings();
    } else if (rom_file == "canyon_bomber.bin") {
        new_game_settings = new CanyonBomberSettings();
    } else if (rom_file == "miner_2049r.bin") {
        new_game_settings = new Miner2049RSettings();
    } else if (rom_file == "bowling.bin") {
        new_game_settings = new BowlingSettings();
    } else if (rom_file == "assault.bin") {
        new_game_settings = new AssaultSettings();
    } else if (rom_file == "up_n_down.bin") {
        new_game_settings = new UpNDownSettings();
    } else if (rom_file == "kangaroo.bin") {
        new_game_settings = new KangarooSettings();
    } else if (rom_file == "defender.bin") {
        new_game_settings = new DefenderSettings();
    } else if (rom_file == "name_this_game.bin") {
        new_game_settings = new NameThisGameSettings();
    } else if (rom_file == "pitfall2.bin") {
        new_game_settings = new Pitfall2Settings();
    } else if (rom_file == "private_eye.bin") {
        new_game_settings = new PrivateEyeSettings();
    } else if (rom_file == "demon_attack.bin") {
        new_game_settings = new DemonAttackSettings();
    } else if (rom_file == "elevator_action.bin") {
        new_game_settings = new ElevatorActionSettings();
    } else if (rom_file == "journey_escape.bin") {
        new_game_settings = new JourneyEscapeSettings();
    } else if (rom_file == "phoenix.bin") {
        new_game_settings = new PhoenixSettings();
    } else if (rom_file == "frostbite.bin") {
        new_game_settings = new FrostbiteSettings();
    } else if (rom_file == ".bin") {
        new_game_settings = new FreewaySettings();
    } else if (rom_file == ".bin") {
        new_game_settings = new FreewaySettings();
    } else {
        cerr << "GameSettings::generate_game_Settings_instance: I don't know" <<
                " what game settings to use for " << rom_file << endl;
		cerr << "make sure the ROM is listed in " 
			 << "GameSettings::generate_game_Settings_instance" << endl;
        exit(-1);
    }
    return new_game_settings;
}

/* *********************************************************************
	Extracts the score from RAM, in the case where score is saved as 
	two decimals on two locations of RAM. This tends to be common,
	so I am writing a generic method for it.
 ******************************************************************** */   
int GameSettings::get_two_decimal_score(int lower_index, int higher_index, 
										const IntVect* console_ram) {
	int score = 0;
    int lower_digits_val = (*console_ram)[lower_index];
    int lower_right_digit = lower_digits_val & 15;
    int lower_left_digit = (lower_digits_val - lower_right_digit) >> 4;
    score += ((10 * lower_left_digit) + lower_right_digit);
	if (higher_index < 0) {
		return score;
	}
    int higher_digits_val = (*console_ram)[higher_index];
    int higher_right_digit = higher_digits_val & 15;
    int higher_left_digit = (higher_digits_val - higher_right_digit) >> 4;
    score += ((1000 * higher_left_digit) + 100 * higher_right_digit);
    return score;

}

/* *********************************************************************
	Extracts the score from RAM, in the case where score is saved as 
	three decimals on three locations of RAM. This tends to be common,
	so I am writing a generic method for it.
 ******************************************************************** */   
int GameSettings::get_three_decimal_score(int lower_index, int middle_index,
							int higher_index,
							const IntVect* console_ram) {
	int score = get_two_decimal_score(lower_index, middle_index, console_ram);
	int higher_digits_val = (*console_ram)[higher_index];
    int higher_right_digit = higher_digits_val & 15;
    int higher_left_digit = (higher_digits_val - higher_right_digit) >> 4;
    score += ((100000 * higher_left_digit) + 10000 * higher_right_digit);
    return score;
}


/* *********************************************************************
	Resets the current state of the GameSettings object
 ******************************************************************** */   
void GameSettings::reset_state(void) {
	i_prev_score = 0;
	i_frames_since_last_restart = 0;
}
		
/* *********************************************************************
	Saves the current state of the GameSettings object to the given 
	serializer object
 ******************************************************************** */   
void GameSettings::save_state(Serializer& out) {
    out.putInt(i_prev_score);
    out.putInt(i_frames_since_last_restart);
}

/* *********************************************************************
	Loads the current state of the GameSettings object from the given 
	serializer object
 ******************************************************************** */   
void GameSettings::load_state(Deserializer& in) {
    i_prev_score = (Int32) in.getInt();
    i_frames_since_last_restart = (Int32) in.getInt();
}


FreewaySettings::FreewaySettings() : 
    GameSettings(   "freeway.bin",  //  rom_file,  
                    0,              // uses_screen_matrix, 
                    1,              // _uses_console_ram,  
                    3,              // skip_frames_num, 
                    1,              // delay_after_restart,  
                    UNDEFINED       // first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_UP);
}

float FreewaySettings::get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram) {
        int score = get_two_decimal_score(103, -1, console_ram);
        int reward = score - i_prev_score;
        if (reward < 0) {
            reward = 0;  // sometimes the score is reset
        }
		if (reward > 1) {
			reward = 1;
		}
        i_prev_score = score;
        return (float)reward;
}

bool FreewaySettings::is_end_of_game(const IntMatrix* screen_matrix, 
                            const IntVect* console_ram, 
							int frame_counter) {
	i_frames_since_last_restart++;
	 if (i_frames_since_last_restart > 2000) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
        return true;
    } 
	return false;
}
  
  
SpaceInvadersSettings::SpaceInvadersSettings() : 
    GameSettings(   "spaceinvaders.bin",  //  rom_file,  
                    0,              // uses_screen_matrix, 
                    1,              // _uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,             // delay_after_restart,  
                    PLAYER_A_FIRE   // first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float SpaceInvadersSettings::get_reward( const IntMatrix* screen_matrix, 
                          const IntVect* console_ram) {
    int score = get_two_decimal_score(104, 102, console_ram);
    int reward = score - i_prev_score;
    i_prev_score = score;
    return (float)reward;
}

bool SpaceInvadersSettings::is_end_of_game( const IntMatrix* screen_matrix, 
                                            const IntVect* console_ram, 
											int frame_counter) {
    int reset_val = (*console_ram)[42];
	reset_val = (reset_val - (reset_val & 15)) >> 4;
	if (reset_val == 8) {
		i_prev_score = 0;
		return true; // game is not reset yet
	}
    int new_lives = (*console_ram)[73];
    if (new_lives == 3) {
        return false;
    } else {
		i_prev_score = 0;
        return true;
    }
}

AstrixSettings::AstrixSettings() : 
    GameSettings(   "asterix.bin",  //  rom_file,  
                    false,              // uses_screen_matrix, 
                    true,              // uses_console_ram,  
                    0,              // skip_frames_num, 
                    100,             // delay_after_restart,  
                    PLAYER_A_FIRE   // first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);

}

float AstrixSettings::get_reward(   const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
    int score = get_two_decimal_score(96, 95, console_ram);
    int reward = score - i_prev_score;
    i_prev_score = score;
    return (float)reward;
}

bool AstrixSettings::is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[83];
    int new_lives = byte_val & 15;
    if (new_lives < 3) {
		i_prev_score = 0;
        return true;
    } else {
		return false;
    }            
}

SeaquestSettings::SeaquestSettings() : 
    GameSettings(   "seaquest.bin",  //  rom_file,  
                    false,              // uses_screen_matrix, 
                    true,              // uses_console_ram,  
                    0,              // skip_frames_num, 
                    150,            // delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_FIRE);
}


float SeaquestSettings::get_reward(   const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
    int score = get_two_decimal_score(58,   57, console_ram);
    int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool SeaquestSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[59];
    int new_lives = byte_val & 15;
    if (new_lives < 3) {
        i_prev_score = 0;
        return true;
    } else {
        return false;		
    }            
}

TenniSettings::TenniSettings() : 
    GameSettings(   "tennis.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
}

float TenniSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
    int my_score = (*console_ram)[69];
	int oppt_score = (*console_ram)[70];
	if (my_score > 0) {
		return 1.0;
	} 
	if (oppt_score > 0) {
		return -1.0;
	} 
	return 0.0;
}

bool TenniSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	int my_score = (*console_ram)[69];
	int oppt_score = (*console_ram)[70];
	if (my_score > 0 || oppt_score > 0) {
		return true;
	} else {
		return false;
	}
}

WizardofWorSettings::WizardofWorSettings() : 
    GameSettings(   "wizard_of_wor.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    100,				// delay_after_restart,  
                    PLAYER_A_UP		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
	
}

float WizardofWorSettings::get_reward(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram) {
    int score = get_two_decimal_score(6, 8, console_ram);
	score -= 8000;
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool WizardofWorSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[13];
    int new_lives = byte_val & 15;
    if (new_lives < 2) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}


KrullSettings::KrullSettings() : 
    GameSettings(   "krull.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    200,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
	
}

float KrullSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = get_two_decimal_score(30, 29, console_ram);
	score -= 8000;
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool KrullSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[31];
    int new_lives = byte_val & 15;
    if (new_lives < 2) {
		i_prev_score = 0;
        return true;
    } else {
			assert(new_lives == 2);
        return false;
    } 
}

AtlantisSettings::AtlantisSettings() : 
    GameSettings(   "atlantis.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float AtlantisSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = get_two_decimal_score(34, 35, console_ram);
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool AtlantisSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 10000) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
}

HEROSettings::HEROSettings() : 
    GameSettings(   "hero.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float HEROSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = get_three_decimal_score(57, 56, 55, console_ram);
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool HEROSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[51];
    int new_lives = byte_val & 15;
    if (new_lives < 4) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}

RiverRaidSettings::RiverRaidSettings() : 
    GameSettings(   "riverraid.bin",//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    150,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
	m_ram_vals_to_digits[0]  = 0;
	m_ram_vals_to_digits[8]	 = 1;
	m_ram_vals_to_digits[16] = 2;
	m_ram_vals_to_digits[24] = 3;
	m_ram_vals_to_digits[32] = 4;
	m_ram_vals_to_digits[40] = 5;
	m_ram_vals_to_digits[48] = 6;
	m_ram_vals_to_digits[56] = 7;
	m_ram_vals_to_digits[64] = 8;
	m_ram_vals_to_digits[72] = 9;

}

float RiverRaidSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = 0;
	int digit = m_ram_vals_to_digits[(*console_ram)[87]];
	score += digit;
	digit = m_ram_vals_to_digits[(*console_ram)[85]];
	score += 10 * digit;
	digit = m_ram_vals_to_digits[(*console_ram)[83]];
	score += 100 * digit;
	digit = m_ram_vals_to_digits[(*console_ram)[81]];
	score += 1000 * digit;
	digit = m_ram_vals_to_digits[(*console_ram)[79]];
	score += 10000 * digit;
	digit = m_ram_vals_to_digits[(*console_ram)[77]];
	score += 100000 * digit;
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool RiverRaidSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[64];
    int new_lives = m_ram_vals_to_digits[byte_val];
    if (new_lives < 3) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}

VentureSettings::VentureSettings() : 
    GameSettings(   "venture.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
}

float VentureSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = get_two_decimal_score(72, 71, console_ram);
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool VentureSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[70];
    int new_lives = byte_val & 15;
    if (new_lives < 3) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}

CrazyClimberSettings::CrazyClimberSettings() : 
    GameSettings(   "crazy_climber.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
}

float CrazyClimberSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = 0;
    int digit = (*console_ram)[2];
	score += digit;
	digit = (*console_ram)[3];
	score += 10 * digit;
	digit = (*console_ram)[4];
	score += 100 * digit;
	digit = (*console_ram)[5];
	score += 1000 * digit;
	float reward = (float)(score - i_prev_score);
	if (reward < 0) {
		reward = 0;
	}
	
    i_prev_score = score;
	return reward;
}

bool CrazyClimberSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[122];
    int new_lives = byte_val & 15;
    if (new_lives < 5) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}

FrontLineSettings::FrontLineSettings() : 
    GameSettings(   "frontline.bin",//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    100,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNLEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNRIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPRIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPLEFTFIRE);

}

float FrontLineSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
    int score = get_two_decimal_score(5, 3, console_ram);
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool FrontLineSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    return false; // don't know how to do this :(
}

ChopperCommandSettings::ChopperCommandSettings() : 
    GameSettings(   "chopper_command.bin",//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    100,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
}

float ChopperCommandSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
    int score = get_two_decimal_score(110, 108, console_ram);
	int reward = score - i_prev_score;
    i_prev_score = score;
	return (float)reward;
}

bool ChopperCommandSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[100];
    int new_lives = byte_val & 15;
    if (new_lives < 3) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}


IceHockeySettings::IceHockeySettings() : 
    GameSettings(   "ice_hockey.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
}

float IceHockeySettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
    int my_score = (*console_ram)[10];
	int oppt_score = (*console_ram)[11];
	my_score = max(my_score, 0);
	oppt_score = max(oppt_score, 0);
 	int score = my_score - oppt_score;
	float reward = (float)(score - i_prev_score);
	if (reward > 1.0) {
		reward = 1.0;
	}
	i_prev_score = score;
	return reward;
}

bool IceHockeySettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 4500) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
}


MontezumaRevengeSettings::MontezumaRevengeSettings() : 
    GameSettings(   "montezuma_revenge.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNLEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNRIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPRIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPLEFTFIRE);
}

float MontezumaRevengeSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_two_decimal_score(20, 19, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool MontezumaRevengeSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[58];
    int new_lives = byte_val & 15;
    if (new_lives < 5) {
		i_prev_score = 0;
        return true;
    } else {
        return false;
    } 
}



GravitarSettings::GravitarSettings() : 
    GameSettings(   "gravitar.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    200,			// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);

}

float GravitarSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(9, 8, 7, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool GravitarSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[4];
    int new_lives = byte_val & 15;
	if (new_lives == 0) {
		// the game hasn't started yet!
		return false;
	}
    if (new_lives < 5) {
		i_prev_score = 0;
        return true;
    } 
	return false;
     
}


CarnivalSettings::CarnivalSettings() : 
    GameSettings(   "carnival.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
}

float CarnivalSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_two_decimal_score(46, 45, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool CarnivalSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[3];
    if (byte_val < 1) {
		i_prev_score = 0;
        return true;
    } 
	return false;
     
}

BeamRiderSettings::BeamRiderSettings() : 
    GameSettings(   "beam_rider.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    PLAYER_A_UP	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
}

float BeamRiderSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(9, 10, 11, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool BeamRiderSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[5];
	if (byte_val == 255) {
		// game has not been reset yet
		i_prev_score = 0;
		return true;
	}
	byte_val = byte_val & 15;
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
     
}

BoxingSettings::BoxingSettings() : 
    GameSettings(	"boxing.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
}

float BoxingSettings::get_reward(const IntMatrix* screen_matrix, 
								 const IntVect* console_ram) {
    int my_score = (*console_ram)[18];
	int oppt_score = (*console_ram)[19];
	my_score = max(my_score, 0);
	oppt_score = max(oppt_score, 0);
 	int score = my_score - oppt_score;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool BoxingSettings::is_end_of_game(const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 4500) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
}


BerzerkSettings::BerzerkSettings() : 
    GameSettings(   "berzerk.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    150,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);

}

float BerzerkSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(95, 94, 93, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool BerzerkSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[89];
	byte_val = byte_val & 15;
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


StarGunnerSettings::StarGunnerSettings() : 
    GameSettings(   "star_gunner.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    200,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
}

float StarGunnerSettings::get_reward(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram) {
	int lower_digit = (*console_ram)[3] & 15;
	if (lower_digit == 10) {
		lower_digit = 0;
	}
	int middle_digit = (*console_ram)[4] & 15;
	if (middle_digit == 10) {
		middle_digit = 0;
	}
	int higher_digit = (*console_ram)[5] & 15;
	if (higher_digit == 10) {
		higher_digit = 0;
	}
    
    int score = lower_digit + 10 * middle_digit + 100 * higher_digit;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool StarGunnerSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[7];
	byte_val = byte_val & 15;
    if (byte_val < 5) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

QBertSettings::QBertSettings() : 
    GameSettings(   "qbert.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
}

float QBertSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(91, 90, 89, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool QBertSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    return false; // can't figure our the death from RAM     
}

AmidarSettings::AmidarSettings() : 
    GameSettings(   "amidar.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float AmidarSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_two_decimal_score(89, 90, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool AmidarSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[86];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

VideoPinballSettings::VideoPinballSettings() : 
    GameSettings(   "video_pinball.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    PLAYER_A_DOWN	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
}

float VideoPinballSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(48, 50, 52, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool VideoPinballSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[25];
	byte_val = byte_val & 15;
    if (byte_val > 1) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


GopherSettings::GopherSettings() : 
    GameSettings(   "gopher.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    20,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float GopherSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(50, 49, 48, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool GopherSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[52];
	byte_val = byte_val & 15;
    if (byte_val < 7) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

TimePilotSettings::TimePilotSettings() : 
    GameSettings(   "time_pilot.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    150,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float TimePilotSettings::get_reward(const IntMatrix* screen_matrix, 
									 const IntVect* console_ram) {
	int score = get_two_decimal_score(13, 15, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool TimePilotSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[11];
	byte_val = byte_val & 15;
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}



FishingDerbySettings::FishingDerbySettings() : 
    GameSettings(   "fishing_derby.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
}

float FishingDerbySettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
    int my_score = get_two_decimal_score(61, -1, console_ram);
	int oppt_score = get_two_decimal_score(62, -1, console_ram);
	my_score = max(my_score, 0);
	oppt_score = max(oppt_score, 0);
 	int score = my_score - oppt_score;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool FishingDerbySettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 4500) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
}

BattleZoneSettings::BattleZoneSettings() : 
    GameSettings(   "battle_zone.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float BattleZoneSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int first_val = (*console_ram)[29];
    int first_right_digit = first_val & 15;
    int first_left_digit = (first_val - first_right_digit) >> 4;
	if (first_left_digit == 10)	{
		first_left_digit = 0;
	}
	int second_val = (*console_ram)[30];
    int second_right_digit = second_val & 15;
    int second_left_digit = (second_val - second_right_digit) >> 4;
	if (second_right_digit == 10)	{
		second_right_digit = 0;
	}
	if (second_left_digit == 10)	{
		second_left_digit = 0;
	}
	int score = first_left_digit + 
				10 * second_right_digit + 
				100 * second_left_digit;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool BattleZoneSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[58];
	byte_val = byte_val & 15;
    if (byte_val < 5) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


PitfallSettings::PitfallSettings() : 
    GameSettings(   "pitfall.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {

}

float PitfallSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(87, 86, 85, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool PitfallSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[0];
	byte_val = (byte_val - (byte_val & 15)) >> 4;
    if (byte_val < 10) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

RoboTankSettings::RoboTankSettings() : 
    GameSettings(   "robotank.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float RoboTankSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_two_decimal_score(53, -1, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool RoboTankSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[40];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

JamesBondSettings::JamesBondSettings() : 
    GameSettings(   "jamesbond.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float JamesBondSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_three_decimal_score(92, 93, 94, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool JamesBondSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[111];
	byte_val = byte_val & 15;
    if (byte_val < 5) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

RoadRunnerSettings::RoadRunnerSettings() : 
    GameSettings(   "road_runner.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float RoadRunnerSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int lower_digit = (*console_ram)[73] & 15;
	if (lower_digit == 10) {
		lower_digit = 0;
	}
	int middle_digit = (*console_ram)[74] & 15;
	if (middle_digit == 10) {
		middle_digit = 0;
	}
	int higher_digit = (*console_ram)[75] & 15;
	if (higher_digit == 10) {
		higher_digit = 0;
	}
	int highest_digit = (*console_ram)[76] & 15;
	if (highest_digit == 10) {
		highest_digit = 0;
	}
	
    
    int score = lower_digit + 10 * middle_digit + 100 * higher_digit + 
				highest_digit * 1000;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;

}

bool RoadRunnerSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[68];
	byte_val = byte_val & 15;
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

AstroidsSettings::AstroidsSettings() : 
    GameSettings(   "astroids.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float AstroidsSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_two_decimal_score(62, 61, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool AstroidsSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[60];
	byte_val = (byte_val - (byte_val & 15)) >> 4;
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

TutankhamSettings::TutankhamSettings() : 
    GameSettings(   "tutankham.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float TutankhamSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
	int score = get_two_decimal_score(28, 26, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool TutankhamSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[30];
	byte_val = (byte_val & 3);
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


EnduroSettings::EnduroSettings() : 
    GameSettings(   "enduro.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float EnduroSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = get_two_decimal_score(43, 44, console_ram);
	float reward = (float)(i_prev_score - score);
	i_prev_score = score;
	return reward;
}

bool EnduroSettings::is_end_of_game(const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 4500) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
}

PooyanSettings::PooyanSettings() : 
    GameSettings(   "pooyan.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float PooyanSettings::get_reward(const IntMatrix* screen_matrix, 
								 const IntVect* console_ram) {
	int score = get_three_decimal_score(10, 9, 8, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool PooyanSettings::is_end_of_game(const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[22];
	byte_val = (byte_val & 15);
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

AirRaidSettings::AirRaidSettings() : 
    GameSettings(   "air_raid.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float AirRaidSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(42, 41, 40, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool AirRaidSettings::is_end_of_game(const IntMatrix* screen_matrix, 
									 const IntVect* console_ram, 
									 int frame_counter) {
    int byte_val = (*console_ram)[39];
	byte_val = (byte_val & 15);
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

AlienSettings::AlienSettings() : 
    GameSettings(   "alien.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float AlienSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = (*console_ram)[91];
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool AlienSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[64];
	byte_val = (byte_val & 15);
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

CentipedeSettings::CentipedeSettings() : 
    GameSettings(   "centipede.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float CentipedeSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = get_three_decimal_score(118, 117, 116, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool CentipedeSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[109];
	byte_val = (byte_val - (byte_val & 15)) >> 4;
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

SolarisSettings::SolarisSettings() : 
    GameSettings(   "solaris.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float SolarisSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = get_two_decimal_score(92, 93, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool SolarisSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[89];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

BankHeistSettings::BankHeistSettings() : 
    GameSettings(   "bank_heist.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float BankHeistSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = get_three_decimal_score(90, 89, 88, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool BankHeistSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[85];
	byte_val = byte_val & 15;
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

ZaxxonSettings::ZaxxonSettings() : 
    GameSettings(   "zaxxon.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float ZaxxonSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = get_two_decimal_score(105, 104, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool ZaxxonSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[106];
	byte_val = byte_val & 15;
    if (byte_val < 5) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

MsPacman::MsPacman() : 
    GameSettings(   "ms_pacman.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    150,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);

}

float MsPacman::get_reward(const IntMatrix* screen_matrix, 
							const IntVect* console_ram) {
	int score = get_three_decimal_score(120, 121, 122, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool MsPacman::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[123];
	byte_val = byte_val & 15;
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


SkiingSettings::SkiingSettings() : 
    GameSettings(   "skiing.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
}

float SkiingSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int score = get_two_decimal_score(107, -1, console_ram);
	int reward = i_prev_score - score;
	if (reward < 0) {
		reward = 0;
	}
    i_prev_score = score;
	return (float)reward;
}

bool SkiingSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 2500) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
}

DoubleDunkSettings::DoubleDunkSettings() : 
    GameSettings(   "double_dunk.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_RIGHTFIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float DoubleDunkSettings::get_reward(	const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram) {
    int my_score = (*console_ram)[118];
	int oppt_score = (*console_ram)[119];
	int score = my_score - oppt_score;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool DoubleDunkSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 1000) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
		
}

JoustSettings::JoustSettings() : 
    GameSettings(   "joust.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float JoustSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = get_three_decimal_score(7, 5, 3, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool JoustSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[1];
	byte_val = byte_val & 15;
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

KungFuMasterSettings::KungFuMasterSettings() : 
    GameSettings(   "kung_fu_master.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    300,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);

}

float KungFuMasterSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
	int score = get_two_decimal_score(25, 24, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool KungFuMasterSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[29];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

CanyonBomberSettings::CanyonBomberSettings() : 
    GameSettings(   "canyon_bomber.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,			// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float CanyonBomberSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int my_score = get_two_decimal_score(75, 74, console_ram);
	int oppt_score = get_two_decimal_score(73, 72, console_ram);;
	int score = my_score - oppt_score;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;
}

bool CanyonBomberSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	i_frames_since_last_restart++;
	if (i_frames_since_last_restart > 3000) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
		return true;
	}
	return false;
		
}

Miner2049RSettings::Miner2049RSettings() : 
    GameSettings(   "miner_2049r.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    100,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_FIRE);
	pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    
}

float Miner2049RSettings::get_reward(const IntMatrix* screen_matrix, 
								const IntVect* console_ram) {
    int digit1 = (*console_ram)[35];
	int digit2 = (*console_ram)[34];
	int digit3 = (*console_ram)[33];
	int digit4 = (*console_ram)[32];
	int score = digit1 + 10 * digit2 + 100 * digit3 + 1000 * digit4;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool Miner2049RSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[79];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

BowlingSettings::BowlingSettings() : 
    GameSettings(   "bowling.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_FIRE);    
}

float BowlingSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_two_decimal_score(33, 38, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool BowlingSettings::is_end_of_game(const IntMatrix* screen_matrix, 
									 const IntVect* console_ram, 
									 int frame_counter) {
    int byte_val = (*console_ram)[36];
	byte_val = byte_val & 15;
    if (byte_val > 10) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

AssaultSettings::AssaultSettings() : 
    GameSettings(   "assault.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
	pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    
}

float AssaultSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(2, 1, 0, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool AssaultSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[101];
	byte_val = byte_val & 15;
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

UpNDownSettings::UpNDownSettings() : 
    GameSettings(   "up_n_down.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
	pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float UpNDownSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(2, 1, 0, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool UpNDownSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[6];
	byte_val = byte_val & 15;
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

KangarooSettings::KangarooSettings() : 
    GameSettings(   "kangaroo.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    100,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_UPRIGHT);
    pv_possible_actions->push_back(PLAYER_A_UPLEFT);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float KangarooSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_two_decimal_score(40, 39, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool KangarooSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[45];
	byte_val = byte_val & 15;
    if (byte_val < 2) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

DefenderSettings::DefenderSettings() : 
    GameSettings(   "defender.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
    pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float DefenderSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int digit1 = (*console_ram)[28] & 15;
	if (digit1 == 10) {
		digit1 = 0;
	}
	int digit2 = (*console_ram)[29] & 15;
	if (digit2 == 10) {
		digit2 = 0;
	}
	int digit3 = (*console_ram)[30] & 15;
	if (digit3 == 10) {
		digit3 = 0;
	}
	int digit4 = (*console_ram)[31] & 15;
	if (digit4 == 10) {
		digit4 = 0;
	}
	int score = digit1 + 10 * digit2 + 100 * digit3 + 1000 * digit4;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool DefenderSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram, 
									int frame_counter) {
    int byte_val = (*console_ram)[66];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

NameThisGameSettings::NameThisGameSettings() : 
    GameSettings(   "name_this_game.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    300,			// delay_after_restart,  
                    PLAYER_A_FIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float NameThisGameSettings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(70, 69, 68, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool NameThisGameSettings::is_end_of_game(	const IntMatrix* screen_matrix, 
											const IntVect* console_ram, 
											int frame_counter) {
    int byte_val = (*console_ram)[71];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


Pitfall2Settings::Pitfall2Settings() : 
    GameSettings(   "pitfall2.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    300,			// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
}

float Pitfall2Settings::get_reward(	const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(73, 72, 71, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool Pitfall2Settings::is_end_of_game(	const IntMatrix* screen_matrix, 
											const IntVect* console_ram, 
											int frame_counter) {
	return false; // this game never ends!
}

PrivateEyeSettings::PrivateEyeSettings() : 
    GameSettings(   "private_eye.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    300,			// delay_after_restart,  
                    PLAYER_A_FIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
	pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float PrivateEyeSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(74, 73, 72, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool PrivateEyeSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	i_frames_since_last_restart++;
	 if (i_frames_since_last_restart > 3600) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
        return true;
    } 
	return false;
	
}

DemonAttackSettings::DemonAttackSettings() : 
    GameSettings(   "demon_attack.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,			// delay_after_restart,  
                    PLAYER_A_FIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float DemonAttackSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(5, 3, 1, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool DemonAttackSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[114];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}

ElevatorActionSettings::ElevatorActionSettings() : 
    GameSettings(   "elevator_action.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    3000,			// delay_after_restart,  
                    PLAYER_A_FIRE		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
	pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_UPFIRE);
    pv_possible_actions->push_back(PLAYER_A_DOWNFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float ElevatorActionSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_two_decimal_score(8, 7, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool ElevatorActionSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[3];
	byte_val = byte_val & 15;
	if (byte_val == 0) {
		// this is the blank screen period, at the begining of the game
		// wait it out
		return false;
	}
    if (byte_val < 4) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


JourneyEscapeSettings::JourneyEscapeSettings() : 
    GameSettings(   "journey_escape.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    30,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
	pv_possible_actions->push_back(PLAYER_A_UP);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
}

float JourneyEscapeSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(18, 17, 16, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool JourneyEscapeSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
	i_frames_since_last_restart++;
	 if (i_frames_since_last_restart > 3600) {
		i_frames_since_last_restart = 0;
		i_prev_score = 0;
        return true;
    } 
	return false;
	
}

PhoenixSettings::PhoenixSettings() : 
    GameSettings(   "phoenix.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    UNDEFINED		// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
    pv_possible_actions->push_back(PLAYER_A_RIGHTFIRE);
    pv_possible_actions->push_back(PLAYER_A_LEFTFIRE);
	pv_possible_actions->push_back(PLAYER_A_FIRE);
}

float PhoenixSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_two_decimal_score(72, 73,console_ram) * 10;
	score += ((*console_ram)[71] - ((*console_ram)[71] & 15)) >> 4;
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool PhoenixSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[75];
	byte_val = byte_val & 15;
    if (byte_val < 5) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}


FrostbiteSettings::FrostbiteSettings() : 
    GameSettings(   "frostbite.bin",	//  rom_file,  
                    false,          // uses_screen_matrix, 
                    true,           // uses_console_ram,  
                    0,              // skip_frames_num, 
                    10,				// delay_after_restart,  
                    PLAYER_A_FIRE	// first_action
                    ) {
    pv_possible_actions->clear();
    pv_possible_actions->push_back(PLAYER_A_NOOP);
    pv_possible_actions->push_back(PLAYER_A_RIGHT);
    pv_possible_actions->push_back(PLAYER_A_LEFT);
    pv_possible_actions->push_back(PLAYER_A_DOWN);
	pv_possible_actions->push_back(PLAYER_A_UP);
}

float FrostbiteSettings::get_reward(const IntMatrix* screen_matrix, 
									const IntVect* console_ram) {
	int score = get_three_decimal_score(74, 73, 72, console_ram);
	float reward = (float)(score - i_prev_score);
	i_prev_score = score;
	return reward;	
}

bool FrostbiteSettings::is_end_of_game(const IntMatrix* screen_matrix, 
										const IntVect* console_ram, 
										int frame_counter) {
    int byte_val = (*console_ram)[76];
	byte_val = byte_val & 15;
    if (byte_val < 3) {
		i_prev_score = 0;
        return true;
    } 
	return false;
}
