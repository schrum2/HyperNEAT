#include "test.h"

string str_ver = "0.1";
string str_welcome = "A.L.E: Atari 2600 Learning Environment (version " + str_ver + ")\n"
+ "[Powered by Stella]\n"
+ "Use -help for help screen.";
OSystem* theOSystem = (OSystem*) NULL;
GameController* p_game_controllr  = NULL;

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
}

void display_screen(const IntMatrix& pm_screen_matrix) {
  // Display screen
  static long long int frame = 0;
  ostringstream filename;
  char buffer [50];

  sprintf (buffer, "%09lld", frame++);
  filename << "exported_screens/toDisplay_frame_" << buffer << ".png";

  theOSystem->p_export_screen->save_png(&pm_screen_matrix, filename.str());
  theOSystem->p_display_screen->display_png(filename.str());
  remove(filename.str().c_str());
};

// int test(int argc, char* argv[]) {
//   initializeEmulator();

//   // Media assets
//   MediaSource& mediasrc = theOSystem->console().mediaSource();
//   int screen_width = mediasrc.width();
//   int screen_height = mediasrc.height();
//   IntMatrix pm_screen_matrix; // 2D Matrix containing screen pixel colors
//   for (int i=0; i<screen_height; ++i) { // Initialize our matrix
//     IntVect row;
//     for (int j=0; j<screen_width; ++j)
//       row.push_back(-1);
//     pm_screen_matrix.push_back(row);
//   }

//   InternalController* controller = (InternalController*) theOSystem->getGameController();
//   SelfDetectionAgent* self_detection_agent = (SelfDetectionAgent*) controller->getPlayerAgentLeft();
//   GameSettings* game_settings = controller->getGameSettings();

//   // Main Loop
//   int skip_frames_num = game_settings->i_skip_frames_num;
//   int frame_skip_ctr = 0;
//   Action action = RESET;
//   time_start = time(NULL);
//   for (int frame=0; frame<100000; frame++) {
//     if (frame_skip_ctr++ >= skip_frames_num) {
//       frame_skip_ctr = 0;

//       // Get the latest screen
//       int ind_i, ind_j;
//       uInt8* pi_curr_frame_buffer = mediasrc.currentFrameBuffer();
//       for (int i = 0; i < screen_width * screen_height; i++) {
//         uInt8 v = pi_curr_frame_buffer[i];
//         ind_i = i / screen_width;
//         ind_j = i - (ind_i * screen_width);
//         pm_screen_matrix[ind_i][ind_j] = v;
//       }

//       // Get the object representation
//       self_detection_agent->process_image(&pm_screen_matrix, action);

//       // Choose Action
//       if (frame <5) action = RESET;
//       else action = PLAYER_A_UP;

//       // Display the screen
//       display_screen(pm_screen_matrix);
//     }

//     // Apply action to simulator and update the simulator
//     theOSystem->applyAction(action);

//     if (frame % 1000 == 0) {
//       time_end = time(NULL);
//       double avg = ((double)frame)/(time_end - time_start);
//       cerr << "Average main loop iterations per sec = " << avg << endl;
//     }
//   }

//   cleanup();
//   return 0;
// }
