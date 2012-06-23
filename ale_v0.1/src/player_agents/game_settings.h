/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  game_settings.h
 *  stella_agent
 *
 *  The implementation of the GameSettings class, which is the superclass for
 *  other game-settings classes. These classes contain game-specific information
 *  and functionality. 
 **************************************************************************** */
#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H


#include "common_constants.h"
#include "../emucore/Serializer.hxx"
#include "../emucore/Deserializer.hxx"
#include <map>

class GameSettings {
    /* *************************************************************************
        This is the superclass for other game-settings classes.
        These classes contain game-specific data and functionality, such
        as how to determine the reward from the game, or how to tell
        when the game has ended
        
        Instance Variables:
            - rom_file              The location of the game rom file
            - pv_possible_actions   List of allowed actions in the game
            - uses_screen_matrix    True means that this games requires the
                                    updated screen-matrix, for either reward
                                    or end-of-game extraction
            - uses_console_ram      True means that this games requires the
                                    updated console RAM content, for either 
                                    reward or end-of-game extraction
            - skip_frames_num       We skip this many frames, after seeing a
                                    frame of the game. Skipping is done to
                                    increase the speed of the simulations
            - delay_after_restart   Delay this number of frames after restarting
                                    Some games do not update the game-state
                                    (either the screen or the RAM values)
                                    for some frames after restarting.
            - first_action          Some games need a specical action at the very
                                    beginning, just to start the game. None means
                                    the game requires no such special action
			- i_prev_score;			Score in the previous timestep
			- i_frames_since_last_restart Number of frames since last reset

    ************************************************************************* */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        GameSettings(   string _rom_file, 
                        bool _uses_screen_matrix, bool _uses_console_ram,
                        int _skip_frames_num, int _delay_after_restart, 
                        Action _first_action);

        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~GameSettings();
        
        /* *********************************************************************
            Abstract Method: Returns the reward, based on either the current 
            screen matrix or the contents of the console RAM, or both.
         ******************************************************************** */
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram) = 0;

        /* *********************************************************************
            Abstract Method: Returns true if we are currently at the end of the 
            game (based on  either the current screen matrix or the contents of 
            the console RAM)
         ******************************************************************** */        
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter) = 0;

		
		/* *********************************************************************
			Resets the current state of the GameSettings object
		 ******************************************************************** */   
		virtual void reset_state(void);
		
		/* *********************************************************************
			Saves the current state of the GameSettings object to the given 
			serializer object
		 ******************************************************************** */   
		virtual void save_state(Serializer& out);

		/* *********************************************************************
			Loads the current state of the GameSettings object from the given 
			serializer object
		 ******************************************************************** */   
		virtual void load_state(Deserializer& in);

        /* *********************************************************************
            Generates an instance of one of the GameSettings subclasses, based 
            on the name of the current ROM file.
            Note 1: If you add a new GameSettngs subclass, you need to also 
                    update it here
            Note 2: The caller is resposible for deleting the returned pointer
        ******************************************************************** */
        static GameSettings* generate_game_Settings_instance(string rom_file);

    // Instance Variables (everything is public! :P)
        string s_rom_file;           // The location of the game rom file
        ActionVect* pv_possible_actions;//List of allowed actions in the game
        bool b_uses_screen_matrix;  // True means that this games requires the
                                    // updated screen-matrix, for either reward
                                    // or end-of-game extraction
        bool b_uses_console_ram;    // True means that this games requires the
                                    // updated console RAM content, for either 
                                    // reward or end-of-game extraction
        int i_skip_frames_num;      // We skip this many frames, after seeing a
                                    // frame of the game. Skipping is done to
                                    // increase the speed of the simulations
        int i_delay_after_restart;  // Delay this number of frames after 
                                    // restarting Some games do not update the 
                                    // game-state(either the screen or the RAM 
                                    // values) for some frames after restarting.
        Action e_first_action;      // Some games need a specical action at the 
                                    // very beginning, just to start the game. 
                                    // None means the game requires no such 
                                    // special action
        int i_prev_score;			// Score in the previous timestep
		int i_frames_since_last_restart; // Number of frames since last reset

	protected:
		/* *********************************************************************
            Extracts the score from RAM, in the case where score is saved as 
			two decimals on two locations of RAM. This tends to be common,
			so I am writing a generic method for it.
         ******************************************************************** */   
		int get_two_decimal_score(	int lower_index, int higher_index, 
									const IntVect* console_ram);

		/* *********************************************************************
            Extracts the score from RAM, in the case where score is saved as 
			three decimals on three locations of RAM. This tends to be common,
			so I am writing a generic method for it.
         ******************************************************************** */   
		int get_three_decimal_score(int lower_index, int middle_index,
									int higher_index,
									const IntVect* console_ram);

};



class FreewaySettings : public GameSettings {    
    public: 
        FreewaySettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};


class SpaceInvadersSettings : public GameSettings {
    public: 
        SpaceInvadersSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AstrixSettings : public GameSettings {
    public: 
        AstrixSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class SeaquestSettings : public GameSettings {
    public: 
        SeaquestSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class TenniSettings : public GameSettings {
    public: 
        TenniSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};


class WizardofWorSettings : public GameSettings {
    public: 
        WizardofWorSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class KrullSettings : public GameSettings {
    public: 
        KrullSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AtlantisSettings : public GameSettings {
    public: 
        AtlantisSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class HEROSettings : public GameSettings {
    public: 
        HEROSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class RiverRaidSettings : public GameSettings {
    public: 
        RiverRaidSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
	private:
		map<int, int> m_ram_vals_to_digits;	// RiverRaid is messed up.
											// It stores the score in a crazy
											// crypted way
};

class VentureSettings : public GameSettings {
    public: 
        VentureSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class CrazyClimberSettings : public GameSettings {
    public: 
        CrazyClimberSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class FrontLineSettings : public GameSettings {
    public: 
        FrontLineSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class ChopperCommandSettings : public GameSettings {
    public: 
        ChopperCommandSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class IceHockeySettings : public GameSettings {
    public: 
        IceHockeySettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class MontezumaRevengeSettings : public GameSettings {
    public: 
        MontezumaRevengeSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class GravitarSettings : public GameSettings {
    public: 
        GravitarSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class CarnivalSettings : public GameSettings {
    public: 
        CarnivalSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class BeamRiderSettings : public GameSettings {
    public: 
        BeamRiderSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class BoxingSettings : public GameSettings {
    public: 
        BoxingSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class BerzerkSettings : public GameSettings {
    public: 
        BerzerkSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class StarGunnerSettings : public GameSettings {
    public: 
        StarGunnerSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class QBertSettings : public GameSettings {
    public: 
        QBertSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AmidarSettings : public GameSettings {
    public: 
        AmidarSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class VideoPinballSettings : public GameSettings {
    public: 
        VideoPinballSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class GopherSettings : public GameSettings {
    public: 
        GopherSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class TimePilotSettings : public GameSettings {
    public: 
        TimePilotSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class FishingDerbySettings : public GameSettings {
    public: 
        FishingDerbySettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class BattleZoneSettings : public GameSettings {
    public: 
        BattleZoneSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class PitfallSettings : public GameSettings {
    public: 
        PitfallSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class RoboTankSettings : public GameSettings {
    public: 
        RoboTankSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class JamesBondSettings : public GameSettings {
    public: 
        JamesBondSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class RoadRunnerSettings : public GameSettings {
    public: 
        RoadRunnerSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AstroidsSettings : public GameSettings {
    public: 
        AstroidsSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class TutankhamSettings : public GameSettings {
    public: 
        TutankhamSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class EnduroSettings : public GameSettings {
    public: 
        EnduroSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class PooyanSettings : public GameSettings {
    public: 
        PooyanSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AirRaidSettings : public GameSettings {
    public: 
        AirRaidSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AlienSettings : public GameSettings {
    public: 
        AlienSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class CentipedeSettings : public GameSettings {
    public: 
        CentipedeSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class SolarisSettings : public GameSettings {
    public: 
        SolarisSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class BankHeistSettings : public GameSettings {
    public: 
        BankHeistSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class ZaxxonSettings : public GameSettings {
    public: 
        ZaxxonSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class MsPacman : public GameSettings {
    public: 
        MsPacman();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class SkiingSettings : public GameSettings {
    public: 
        SkiingSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class DoubleDunkSettings : public GameSettings {
    public: 
        DoubleDunkSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class JoustSettings : public GameSettings {
    public: 
        JoustSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class KungFuMasterSettings : public GameSettings {
    public: 
        KungFuMasterSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class CanyonBomberSettings : public GameSettings {
    public: 
        CanyonBomberSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class Miner2049RSettings : public GameSettings {
    public: 
        Miner2049RSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class BowlingSettings : public GameSettings {
    public: 
        BowlingSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class AssaultSettings : public GameSettings {
    public: 
        AssaultSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class UpNDownSettings : public GameSettings {
    public: 
        UpNDownSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class KangarooSettings : public GameSettings {
    public: 
        KangarooSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class DefenderSettings : public GameSettings {
    public: 
        DefenderSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class NameThisGameSettings : public GameSettings {
    public: 
        NameThisGameSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class Pitfall2Settings : public GameSettings {
    public: 
        Pitfall2Settings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class PrivateEyeSettings : public GameSettings {
    public: 
        PrivateEyeSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class DemonAttackSettings : public GameSettings {
    public: 
        DemonAttackSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class ElevatorActionSettings : public GameSettings {
    public: 
        ElevatorActionSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class JourneyEscapeSettings : public GameSettings {
    public: 
        JourneyEscapeSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class PhoenixSettings : public GameSettings {
    public: 
        PhoenixSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

class FrostbiteSettings : public GameSettings {
    public: 
        FrostbiteSettings();
        virtual float get_reward( const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram);
        virtual bool is_end_of_game(const IntMatrix* screen_matrix, 
                                    const IntVect* console_ram, 
									int frame_counter);
};

#endif
