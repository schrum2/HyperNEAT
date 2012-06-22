#ifndef ATARI_INTERFACE_H
#define ATARI_INTERFACE_H

#include <cstdlib>
#include <ctime> 
#include "bspf.hxx"
#include "Console.hxx"
#include "Event.hxx"
#include "PropsSet.hxx"
#include "Settings.hxx"
#include "FSNode.hxx"
#include "OSystem.hxx"
#include "SettingsUNIX.hxx"
#include "OSystemUNIX.hxx"
#include "fifo_controller.h"
#include "internal_controller.h"
#include "common_constants.h"
#include "game_settings.h"
#include "player_agent.h"
#include "self_detection_agent.h"

string str_ver = "0.1";
string str_welcome = "A.L.E: Atari 2600 Learning Environment (version " + str_ver + ")\n"
+ "[Powered by Stella]\n"
+ "Use -help for help screen.";
OSystem* theOSystem = (OSystem*) NULL;
InternalController* p_game_controllr  = NULL;

static size_t time_start; 
static size_t time_end; 

void cleanup() {
  // Does general Cleanup in case any operation failed (or at end of program)
  if(theOSystem) {
    delete theOSystem;
  }

  if(p_game_controllr) {
    delete p_game_controllr;
  }
}

void initializeEmulator(string rom_file, bool display_screen) {
  int argc = 6;
  char** argv = new char*[argc];
  for (int i=0; i<=argc; i++) {
    argv[i] = new char[200];
  }
  strcpy(argv[0],"./ale");
  strcpy(argv[1],"-player_agent");
  strcpy(argv[2],"self_detection_agent");
  strcpy(argv[3],"-display_screen");
  if (display_screen)
    strcpy(argv[4],"true");
  else
    strcpy(argv[4],"false");

  //strcpy(argv[5],"../ale_v0.1/roms/asterix.bin");
  strcpy(argv[5],rom_file.c_str());  

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
    return;
  } else if(theOSystem->createConsole(romfile)) 	{
    printf("Running ROM file...\n");
    theOSystem->settings().setString("rom_file", romfile);
  } else {
    cleanup();
    return;
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
  p_game_controllr = new InternalController(theOSystem);
  theOSystem->setGameController(p_game_controllr);
  cout << "Games will be controlled internally, " << 
      "through the assigned player Agent" << endl;

  // Set the Pallete 
  theOSystem->console().setPalette("standard");
}

void display_screen(const IntMatrix& pm_screen_matrix) {
    p_game_controllr->getPlayerAgentLeft()->display_screen(pm_screen_matrix);
};

#endif
