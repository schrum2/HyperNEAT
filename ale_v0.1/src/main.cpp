/* *****************************************************************************
 *  main.cpp
 *  
 *  The entry point for stella_agent (this replaces mainSDK.cxx)
 *
 **************************************************************************** */
#include <cstdlib>
#include <ctime> 
#include "emucore/m6502/src/bspf/src/bspf.hxx"
#include "emucore/Console.hxx"
#include "emucore/Event.hxx"
#include "emucore/PropsSet.hxx"
#include "emucore/Settings.hxx"
#include "emucore/FSNode.hxx"
#include "emucore/OSystem.hxx"
#include "os_dependent/SettingsUNIX.hxx"
#include "os_dependent/OSystemUNIX.hxx"
#include "control/fifo_controller.h"
#include "control/internal_controller.h"
#include "player_agents/common_constants.h"

string str_ver = "0.1";
string str_welcome = "A.L.E: Atari 2600 Learning Environment (version " + str_ver + ")\n"
					+ "[Powered by Stella]\n"
					+ "Use -help for help screen.";
OSystem* theOSystem = (OSystem*) NULL;
GameController* p_game_controllr  = NULL;

void Cleanup() {
	// Does general Cleanup in case any operation failed (or at end of program)
	if(theOSystem) {
		delete theOSystem;
	}
    
    if(p_game_controllr) {
        delete p_game_controllr;
    }
}


int main(int argc, char* argv[]) {
	cout << str_welcome << endl;
	theOSystem = new OSystemUNIX();
	SettingsUNIX settings(theOSystem);
	theOSystem->settings().loadConfig();
		
	// Load the RL parameters
    string rl_params_loc = theOSystem->settings().getString("working_dir") +
                            theOSystem->settings().getString("rl_params_file");
	theOSystem->settings().loadConfig(rl_params_loc.c_str());

	// Load the Class Discovery parameters
    string cl_dis_params_loc = theOSystem->settings().getString("working_dir") +
					theOSystem->settings().getString("class_disc_params_file");
	theOSystem->settings().loadConfig(cl_dis_params_loc.c_str());

	// Load the Search-Agent parameters
    string search_params_loc = theOSystem->settings().getString("working_dir") +
								"search_params.txt";
	theOSystem->settings().loadConfig(search_params_loc.c_str());

	// Load the Experiment parameters
    string exp_params_loc = theOSystem->settings().getString("working_dir") +
							"experiment_params.txt";
	theOSystem->settings().loadConfig(exp_params_loc.c_str());

	// Take care of commandline arguments (over-ride all file settings)
	string romfile = theOSystem->settings().loadCommandLine(argc, argv);

	// Finally, make sure the settings are valid
	// We do it once here, so the rest of the program can assume valid settings
	theOSystem->settings().validate();

					
	// Create the full OSystem after the settings, since settings are
	// probably needed for defaults
	theOSystem->create();
	
	//// Main loop ////
	// First we check if a ROM is specified on the commandline.  If so, and if
	//   the ROM actually exists, use it to create a new console.
	// If not, use the built-in ROM launcher.  In this case, we enter 'launcher'
	//   mode and let the main event loop take care of opening a new console/ROM.
	if(argc == 1 || romfile == "" || !FilesystemNode::fileExists(romfile)) {
		printf("No ROM File specified or the ROM file was not found.\n");
		return -1;
	} else if(theOSystem->createConsole(romfile)) 	{
		printf("Running ROM file...\n");
        theOSystem->settings().setString("rom_file", romfile);
	} else {
		Cleanup();
		return 0;
	}
    
	// Seed the Random number generator
    if (theOSystem->settings().getString("random_seed") == "time") {
        cout << "Random Seed: Time" << endl;
        srand((unsigned)time(0)); 
        srand48((unsigned)time(0));
    } else {
        int seed = theOSystem->settings().getInt("random_seed");
        assert(seed >= 0);
        cout << "Random Seed: " << seed << endl;
        srand((unsigned)seed); 
        srand48((unsigned)seed);
    }
    
    // Generate the GameController
    if (theOSystem->settings().getString("game_controller") == "fifo") {
        p_game_controllr = new FIFOController(theOSystem);
        theOSystem->setGameController(p_game_controllr);
        cout << "Games will be controlled trhough FIFO pipes." << endl;
    } else {
        p_game_controllr = new InternalController(theOSystem);
        theOSystem->setGameController(p_game_controllr);
        cout << "Games will be controlled internally, " << 
                "through the assigned player Agent" << endl;
    }
    // Set the Pallete 
    theOSystem->console().setPalette("standard");
    
    
	
	
	// Start the main loop, and don't exit until the user issues a QUIT command
	theOSystem->mainLoop();
	
	// Cleanup time ...
	Cleanup();
	return 0;
}

