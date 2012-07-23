#ifndef ALE_INTERFACE_H
#define ALE_INTERFACE_H

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
#include "player_agents/game_settings.h"
#include "player_agents/player_agent.h"
#include "player_agents/self_detection_agent.h"

/**
   This class interfaces ALE with external code for controlling agents.
 */
class ALEInterface
{
public:
    OSystem* theOSystem;
    InternalController* game_controller;

    MediaSource *mediasrc;
    int screen_width, screen_height;
    IntMatrix screen_matrix;
    IntVect ram_content;

    System* emulator_system;
    GameSettings* game_settings;

    int skip_frames_num;         // Indicates if we take actions on each timestep or skip some frames
    int frame;                   // Current frame number
    int max_num_frames;          // Maximum number of frames allowed in this episode
    float game_score;            // Score accumulated throughout the course of a game
    ActionVect *allowed_actions; // Vector of allowed actions for this game
    Action last_action;          // Always stores the latest action taken
    time_t time_start, time_end; // Used to keep track of fps
    bool display_active;         // Should the screen be displayed or not

public:
    ALEInterface(): theOSystem(NULL), game_controller(NULL), mediasrc(NULL), emulator_system(NULL),
                    game_settings(NULL), skip_frames_num(0), frame(0), max_num_frames(-1),
                    game_score(0), display_active(false) {}

    ~ALEInterface() {
        if (theOSystem) delete theOSystem;
        if (game_controller) delete game_controller;
    }

    // Loads and initializes a game. After this call the game should be ready to play.
    bool loadROM(string rom_file, bool display_screen) {
        display_active = display_screen;
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

        strcpy(argv[5],rom_file.c_str());  

        string str_ver = "0.1";
        string str_welcome = "A.L.E: Atari 2600 Learning Environment (version " + str_ver + ")\n"
            + "[Powered by Stella]\n"
            + "Use -help for help screen.";

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

        // Get the max number of frames per episode
        max_num_frames = theOSystem->settings().getInt("max_num_frames_per_episode",true);

        //// Main loop ////
        // First we check if a ROM is specified on the commandline.  If so, and if
        //   the ROM actually exists, use it to create a new console.
        // If not, use the built-in ROM launcher.  In this case, we enter 'launcher'
        //   mode and let the main event loop take care of opening a new console/ROM.
        if(argc == 1 || romfile == "" || !FilesystemNode::fileExists(romfile)) {
            printf("No ROM File specified or the ROM file was not found.\n");
            return false;
        } else if(theOSystem->createConsole(romfile)) 	{
            printf("Running ROM file...\n");
            theOSystem->settings().setString("rom_file", romfile);
        } else {
            printf("Unable to create console from ROM file.\n");
            return false;
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
        game_controller = new InternalController(theOSystem);
        theOSystem->setGameController(game_controller);

        // Set the Pallete 
        theOSystem->console().setPalette("standard");

        // Setup the screen representation
        mediasrc = &theOSystem->console().mediaSource();
        screen_width = mediasrc->width();
        screen_height = mediasrc->height();
        for (int i=0; i<screen_height; ++i) { // Initialize our screen matrix
            IntVect row;
            for (int j=0; j<screen_width; ++j)
                row.push_back(-1);
            screen_matrix.push_back(row);
        }

        // Intialize the ram array
        for (int i=0; i<RAM_LENGTH; i++)
            ram_content.push_back(0);

        emulator_system = &theOSystem->console().system();
        game_settings = game_controller->getGameSettings();
        skip_frames_num = game_settings->i_skip_frames_num;
        allowed_actions = game_settings->pv_possible_actions;
    
        reset_game();

        return true;
    }

    // Resets the game
    void reset_game() {
        frame = 0;
        time_start = time(NULL);
        
        for (int i=0; i<5; i++)
            theOSystem->applyAction(RESET); 

        // Apply any necessary actions to start the game
        int delay_after_restart = game_settings->i_delay_after_restart;
        last_action = game_settings->e_first_action;
        if (last_action == UNDEFINED) last_action = PLAYER_A_NOOP;
        for (int i=0; i<delay_after_restart; i++)
            theOSystem->applyAction(last_action);

        // Get the first screen
        int ind_i, ind_j;
        uInt8* pi_curr_frame_buffer = mediasrc->currentFrameBuffer();
        for (int i = 0; i < screen_width * screen_height; i++) {
            uInt8 v = pi_curr_frame_buffer[i];
            ind_i = i / screen_width;
            ind_j = i - (ind_i * screen_width);
            screen_matrix[ind_i][ind_j] = v;
        }

        // Get the first ram content
        for(int i = 0; i<RAM_LENGTH; i++) {
            int offset = i;
            offset &= 0x7f; // there are only 128 bytes
            ram_content[i] = emulator_system->peek(offset + 0x80);
        }
    }

    // Indicates if the game has ended either naturally or due to exceeding
    // the frame limit.
    bool game_over() {
        return game_settings->is_end_of_game(&screen_matrix,&ram_content,frame) ||
            (max_num_frames > -1 && frame >= max_num_frames);
    }

    // Applies an action to the game and returns the reward. It is the user's responsibility
    // to check if the game has ended and reset when necessary -- this method will keep pressing
    // buttons on the game over screen.
    float apply_action(Action action) {
        float action_reward = 0;
        for (int fskip = 0; fskip <= skip_frames_num; fskip++) {
            frame++;
            
            // Apply action to simulator and update the simulator
            theOSystem->applyAction(action);

            // Get the latest screen
            int ind_i, ind_j;
            uInt8* pi_curr_frame_buffer = mediasrc->currentFrameBuffer();
            for (int i = 0; i < screen_width * screen_height; i++) {
                uInt8 v = pi_curr_frame_buffer[i];
                ind_i = i / screen_width;
                ind_j = i - (ind_i * screen_width);
                screen_matrix[ind_i][ind_j] = v;
            }

            // Get the latest ram content
            for(int i = 0; i<RAM_LENGTH; i++) {
                int offset = i;
                offset &= 0x7f; // there are only 128 bytes
                ram_content[i] = emulator_system->peek(offset + 0x80);
            }

            // Get the reward
            action_reward += game_settings->get_reward(&screen_matrix,&ram_content);

            if (frame % 1000 == 0) {
                time_end = time(NULL);
                double avg = ((double)frame)/(time_end - time_start);
                cerr << "Average main loop iterations per sec = " << avg << endl;
            }
        }

        // Display the screen
        if (display_active) {
            theOSystem->p_display_screen->display_screen(screen_matrix, screen_width, screen_height);
        }

        game_score += action_reward;
        last_action = action;
        return action_reward;
    }
};

#endif
