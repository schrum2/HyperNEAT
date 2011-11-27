//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: Settings.cxx,v 1.125 2007/08/22 13:55:40 stephena Exp $
//============================================================================

#include <cassert>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "OSystem.hxx"
#include "Version.hxx"
#include "bspf.hxx"
#include "Settings.hxx"
#include "GuiUtils.hxx"  //ALE 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Settings::Settings(OSystem* osystem) : myOSystem(osystem) {
    // Add this settings object to the OSystem
    myOSystem->attach(this);

    // Add options that are common to all versions of Stella
    setInternal("video", "soft");

    setInternal("gl_filter", "nearest");
    setInternal("gl_aspect", "100");
    setInternal("gl_fsmax", "never");
    setInternal("gl_lib", "libGL.so");
    setInternal("gl_vsync", "false");
    setInternal("gl_texrect", "false");

    setInternal("zoom_ui", "2");
    setInternal("zoom_tia", "2");
    setInternal("fullscreen", "false");
    setInternal("fullres", "");
    setInternal("center", "true");
    setInternal("grabmouse", "false");
    setInternal("palette", "standard");
    setInternal("colorloss", "false");

    setInternal("sound", "false");
    setInternal("fragsize", "512");
    setInternal("freq", "31400");
    setInternal("tiafreq", "31400");
    setInternal("volume", "100");
    setInternal("clipvol", "true");

    setInternal("keymap", "");
    setInternal("joymap", "");
    setInternal("joyaxismap", "");
    setInternal("joyhatmap", "");
    setInternal("paddle", "0");
    setInternal("sa1", "left");
    setInternal("sa2", "right");
    setInternal("p0speed", "50");
    setInternal("p1speed", "50");
    setInternal("p2speed", "50");
    setInternal("p3speed", "50");
    setInternal("pthresh", "600");

    setInternal("showinfo", "false");

    setInternal("ssdir", string(".") + BSPF_PATH_SEPARATOR);
    setInternal("sssingle", "false");

    setInternal("romdir", "");
    setInternal("statedir", "");
    setInternal("cheatfile", "");
    setInternal("palettefile", "");
    setInternal("propsfile", "");
    setInternal("working_dir",  string(".") + BSPF_PATH_SEPARATOR);
    setInternal("rl_params_file",  "rl_params.txt");
    setInternal("class_disc_params_file",  "class_disc_params.txt");
    setInternal("rombrowse", "true");
    setInternal("lastrom", "");

    setInternal("debuggerres", "1030x690");
    setInternal("launcherres", "400x300");
    setInternal("uipalette", "0");
    setInternal("mwheel", "4");
    setInternal("autoslot", "false");
    
    // PLayer Agent Settings
    setInternal("game_controller", "internal");    // Defines how stella will 
                                        // communicate with the player agent:
                                        // - 'internal': an instance of the 
                                        //      PlayerAgent subclass will be 
                                        //      initilized and control the game
                                        // - 'fifo': The control will be through
                                        //      two FIFO pipes, one sending the 
                                        //      screen,ram, and the other 
                                        //      recieving the actions
    
    setInternal("initial_delay", "0"); //"1000"); // How many frames we wait at the very 
    // begining, before resetting the game
    setInternal("random_seed", "0");    // The seed used for random number 
                                        // generation. 'time' will use the 
                                        // current time
    setInternal("export_frames_frq", "0");  // How often to export the frames.
                                            // 0 means never.
    setInternal("display_screen", "false"); // When true we will show the game screen as it goes on.
    setInternal("use_bass", "false");
    
    setInternal("export_frames_after", "0");	// Exporting frames starts after 
    // this many frames
    setInternal("export_frames_before", "-1");  // Exporting frames stops after 
    // this many frames
    setInternal("export_screens_on_last_n_screens", "0");  // Only the last n 
    // screens will be exported
    setInternal("export_screens_on_last_n_episodes", "0");  // Only the last n 
    // episodes will be exported
    setInternal("export_weights_frq", "0"); // How often to export the weights 
                                            // vector. 0 means never.
    setInternal("export_rewards_frq", "0"); // How often to export the rewards 
                                            // history. 0 means never.
    setInternal("plot_class_inst_frq", "0"); // How often to plot the class 
                                             // instances. 0 means never.

    setInternal("export_score_screens", "false"); // When true, we will save the 
    // screens where we recieve non-zero
    // rewards
    setInternal("export_death_screens", "false"); // When true, we will save the 
    // screens where we die

    setInternal("max_num_episodes", "-1");  // The program will quit after this
                                            // number of episodes. -1 = never
    setInternal("max_num_frames", "-1");	// The program will quit after this
    // number of frames. -1 = never
    setInternal("max_num_frames_per_episode", "-1");	// Ends each episode 
    // after this number of frames
    setInternal("num_episodes_to_avg_results", "1000"); // The learning results
    // will be summarized over this many 
    // episodes.
    setInternal("alpha", "-1");				// alpha value for Sarsa-lambda.
    // -1 (default) means alpha will be 
    // selected automatically.
    setInternal("import_weights_file", ""); // When set, we will import the 
    // given weights file.
    setInternal("init_act_explor_count", "0");// At the very begining, we will try
    // repeating each action this many
    // times (this is a mthod for 
    // exploration) Default of 0 means
    // no action exploration.
    setInternal("use_delta_bar_delta", "false"); // When true, we will use 
    // delta-bar-delta to calculate
    // dynamic step sizes in SARSA
    setInternal("use_idbd", "false");			// When true, we will use 
    // iDBD to calculate
    // dynamic step sizes in SARSA
    setInternal("single_action_index", "-1");	// Index of the action that
    // Single-Action Agent will 
    // return
    setInternal("minus_one_zero_reward", "false");	// Use -1/0 reward system. 
    setInternal("end_game_with_score", "false");// When true, the game ends 
    // after each score recieved.
    setInternal("end_episode_with_reward", "false");// When true, the episode 
    // ends with each score 

    setInternal("do_bg_detection", "false");	// When true, the player-agent 
    // will also do backgroudn 
    // detection 	
    setInternal("do_class_disc", "false");		// When true, the player-agent 
    // will also do class discovery
    setInternal("max_perc_difference", "0.1");	// The maximum percentage of 
    // pixels of the two shapes that
    // can be different and the 
    // shapes still be considered
    // similar
    setInternal("max_shape_area_dif", "1.2");	// How much two shape areas can
    // differ, and the distance 
    // between them still be 
    // meaningful. 
    setInternal("max_obj_velocity", "8");       // Maximum velocity (pixel/sec)
    // of objects on screen
    setInternal("num_block_per_row", "10");     // How many blocks per row
    setInternal("num_block_per_col", "10");     // How many blocks per column
    setInternal("do_subtract_background", "true"); // When true we will subtract
    // the background before 
    // generating the feature-vector
    setInternal("plot_gridscr_grids", "false");	// When true,  we will plot the
    // grid extracted for the screen  
    // by gridscr agent.
    setInternal("epsilon_dim_start", "-1");	    // Starts diminishing epsilon 
    // after the give nratio of 
    // episodes. -1 (default) means 
    // never
    setInternal("shrink_weights_frq", "0");		// How often to remove the 
    // smallest value in the weights
    // vactor.
    setInternal("save_model",""); // Filename to save model file to
    setInternal("load_model",""); // Filename to save load model file from
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Settings::~Settings()
{
  myInternalSettings.clear();
  myExternalSettings.clear();
}

void Settings::loadConfig(const char* config_file){
	string line, key, value;
	string::size_type equalPos, garbage;

	ifstream in(config_file);
	if(!in || !in.is_open()) {
	cerr << "Warning: couldn't load settings file: " << config_file << "\n";
	return;
	}

	while(getline(in, line)) {
        // Strip all whitespace and tabs from the line
        while((garbage = line.find("\t")) != string::npos)
          line.erase(garbage, 1);

        // Ignore commented and empty lines
        if((line.length() == 0) || (line[0] == ';'))
          continue;

        // Search for the equal sign and discard the line if its not found
        if((equalPos = line.find("=")) == string::npos)
          continue;

        // Split the line into key/value pairs and trim any whitespace
        key   = line.substr(0, equalPos);
        value = line.substr(equalPos + 1, line.length() - key.length() - 1);
        key   = trim(key);
        value = trim(value);

        // Check for absent key or value
        if((key.length() == 0) || (value.length() == 0))
          continue;

        // Only settings which have been previously set are valid
        //ALE  if(int idx = getInternalPos(key) != -1)
        //ALE  setInternal(key, value, idx, true);
        setInternal(key, value);
	}

	in.close();
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::loadConfig()
{
 loadConfig(myOSystem->configFile().c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Settings::loadCommandLine(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
  {
    // strip off the '-' character
    string key = argv[i];
    if(key[0] == '-')
    {
      key = key.substr(1, key.length());

      // Take care of the arguments which are meant to be executed immediately
      // (and then Stella should exit)
      if(key == "help" || key == "listrominfo")
      {
		usage();
        setExternal(key, "true");
        return "";
      }

      // Take care of arguments without an option
      if(key == "rominfo" || key == "debug" || key == "holdreset" ||
         key == "holdselect" || key == "holdbutton0")
      {
        setExternal(key, "true");
        continue;
      }

      if(++i >= argc)
      {
        cerr << "Missing argument for '" << key << "'" << endl;
        return "";
      }
      string value = argv[i];

      // Settings read from the commandline must not be saved to 
      // the rc-file, unless they were previously set
      if(int idx = getInternalPos(key) != -1)
        setInternal(key, value, idx);   // don't set initialValue here
      else
        setExternal(key, value);
    }
    else
      return key;
  }

  return "";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::validate()
{
  string s;
  int i;

  s = getString("video");
  if(s != "soft" && s != "gl")
    setInternal("video", "soft");

#ifdef DISPLAY_OPENGL
  s = getString("gl_filter");
  if(s != "linear" && s != "nearest")
    setInternal("gl_filter", "nearest");

  i = getInt("gl_aspect");
  if(i < 50 || i > 100)
    setInternal("gl_aspect", "100");

  s = getString("gl_fsmax");
  if(s != "never" && s != "ui" && s != "tia" && s != "always")
    setInternal("gl_fsmax", "never");
#endif

#ifdef SOUND_SUPPORT
  i = getInt("volume");
  if(i < 0 || i > 100)
    setInternal("volume", "100");
  i = getInt("freq");
  if(i < 0 || i > 48000)
    setInternal("freq", "31400");
  i = getInt("tiafreq");
  if(i < 0 || i > 48000)
    setInternal("tiafreq", "31400");
#endif

  i = getInt("zoom_ui");
  if(i < 1 || i > 10)
    setInternal("zoom_ui", "2");

  i = getInt("zoom_tia");
  if(i < 1 || i > 10)
    setInternal("zoom_tia", "2");

  i = getInt("paddle");
  if(i < 0 || i > 3)
    setInternal("paddle", "0");

  i = getInt("pthresh");
  if(i < 400)
    setInternal("pthresh", "400");
  else if(i > 800)
    setInternal("pthresh", "800");

  s = getString("palette");
  if(s != "standard" && s != "z26" && s != "user")
    setInternal("palette", "standard");
	
  // I don't want to export frames during the initial delay
  if ( getInt("export_frames_after", true) == 0) {
	string initial_delay = getString("initial_delay", true);
	setInternal("export_frames_after", initial_delay);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::usage() {

	cout << endl
	<< " *************************************************************************" 		<< endl
	<< " * Welcome to A.L.E (Atari 2600 Learning Environment) "									<< endl
	<< " * (Powered by Stella)"																<< endl
	<< " ***************************************************************************" 		<< endl
    << endl
    << " * Usage: ale [options ...] romfile" << endl
    << endl
    << endl
    << " * Valid options are:" << endl
    << " *   -game_controller [internal]/[fifo]" << endl 
	<< " *    Defines how stella will communicate with the player agent:"					<< endl
	<< " *          - 'internal': (default) an instance of the PlayerAgent"				<< endl 
	<< " *                        subclass will be initilized and control the game"		<< endl
	<< " *           - 'fifo':    The control will be through two FIFO pipes, one sending the "<< endl
	<< " *                        screen,ram, and the other recieving the actions"			<< endl
    << endl
    << " *   -player_agent [bass]/[disco]/[ram]/[random_]/[search_agent]/[single_action]"      << endl 
	<< " * 				/[actions_summary]"												<< endl
	<< " *    Determines what player agent the internal controller should use"				<< endl
    << endl
    << " *  -initial_delay n"																<< endl
	<< " *   How many frames to wait at the very begining, before resetting the game"		<< endl
<< endl
    << " *  -random_seed  [time]/[n] "														<< endl
	<< " *   Sets the seed used for random number generation. "							<< endl 
	<< " *   'time' will use the the current time."										<< endl
<< endl 
    << " *  -export_frames_frq n"															<< endl
	<< " *   How often to export the frames as PNG files. 0 (default) means never."		<< endl
<< endl
    << " *  -export_frames_after n"														<< endl
	<< " *   Exporting frames starts after this many frames. Default is 0, i.e. right away"<< endl
<< endl
    << " *  -export_frames_before n"														<< endl
	<< " *   Exporting frames starts before this many frames. Default is -1, meaning never"<< endl
<< endl
    << " *  -export_screens_on_last_n_screens n"											<< endl
	<< " *   Only the last n screens will be exported. "									<< endl
	<< " *   0 (default) means no change to the current export rules"						<< endl
<< endl
    << " *  -export_screens_on_last_n_episodes n"											<< endl
	<< " *   Only the last n episodes will be exported. "									<< endl
	<< " *   0 (default) means no change to the current export rules"						<< endl
<< endl	
	<< " *  -export_weights_frq n"															<< endl
	<< " *   How often to export the weights vector. 0 (default) means never."				<< endl
<< endl
	<< " *  -export_rewards_frq n"															<< endl
	<< " *   How often to export the rewards history. 0 (default) means never."			<< endl
<< endl
	<< " *  -plot_class_inst_frq f"														<< endl
	<< " *   How often to plot the class instances. 0 (default) means never."				<< endl
<< endl
	<< " *  -export_score_screens [true]/[false]"											<< endl
	<< " *   When true, we will save the screens where the agent recieves a non-zero reward"<< endl
<< endl
	<< " *  -export_death_screens [true]/[false]"											<< endl
	<< " *   When true, we will save the screens where the agent dies"						<< endl
<< endl
	<< " *  -max_num_episodes n"															<< endl
	<< " *   The program will quit after this number of episodes. -1 (default) means never."<< endl
<< endl
	<< " *  -max_num_frames m"																<< endl
	<< " *  The program will quit after this number of frames. -1 (default) means never."	<< endl
<< endl 
	<< " *  -max_num_frames_per_episode m"													<< endl
	<< " *  Ends each episode after this number of frames. -1 (default) means never."		<< endl
<< endl																					
	<< " *  -num_episodes_to_avg_results n"														<< endl
	<< " *   The learning results will be averaged over this number of episodes. "			<< endl
	<< " *   Default is 1000"																<< endl
<< endl 
	<< " *  -import_weights_file str"														<< endl
	<< " *   When set, we will import the given weights file."								<< endl 
<< endl
	<< " *  -ld [A/B]"																		<< endl
	<< " *   Left player difficulty. B (default) means easy"								<< endl
<< endl
	<< " *  -rd [A/B]"																		<< endl
	<< " *   Right player difficulty. B (default) means easy"								<< endl
<< endl
	<< " *  -end_game_with_score [true/false]"												<< endl
	<< " *   When true, the game ends after each score recieved. This is useful in solving "<< endl
	<< " *   some games. Default is false."												<< endl
<< endl
	<< " *  -single_action_index "															<< endl
	<< " *   Index of the action that Single-Action Agent will return"						<< endl
    << endl
	<< " *  -minus_one_zero_reward    Use -1/0 reward system. When true, reward will be -1 on"<< endl
	<< " *                            everysteps, esxcept when score is positive, in which "<< endl
	<< " *                            case the reward will be 0"							<< endl
<< endl
<< endl
	<< " * Sarsa-Lambda Parameters (usually loaded from 'rl_params.txt'). "				<< endl
	<< " * Note that these parameters (except alpha)have no default values"				<< endl
	<< " *  -epsilon f      probability of random action"									<< endl
	<< " *  -lambda f       trace-decay parameter"											<< endl
	<< " *  -gamma f        discount-rate parameter"										<< endl
	<< " *  -optimistic_init [true]/[false]"												<< endl
	<< " *   when true, we will do optimitic initialization"								<< endl
	<< " *  -normalize_feature_vector [true]/[false]"										<< endl
	<< " *   when true, the feature-vector will be normalized to add to 1.0"				<< endl
<< endl
	<< " *  -alpha_multiplier n"															<< endl
	<< " *  -trace_vec_size_ratio f"														<< endl
	<< " *  -minimum_trace_value f"															<< endl
	<< " *  -alpha f"                                                                       << endl
    << " *   alpha value for Sarsa-lambda. -1 (default) means alpha will be"		        << endl	
	<< " *   selected automatically."											            << endl
<< endl
	<< " *  -theta f"                                                                       << endl
    << " *   meta leatning-rate used in iDBD"												<< endl	
<< endl
	<< " *  -shrink_weights_frq n"															<< endl
	<< " *   How often to remove the smallest value in the weights vactor. "				<< endl
	<< " *   0 (default) means never."														<< endl
<< endl
	<< " *  -shrink_weights_method [sqr_root_num_frames/linear_shrink/const_shrink]"		<< endl
	<< " *   Number of values that will be kept in the weights vector is"					<< endl
	<< " *     sqr_root(i_frame_counter) * shrink_weights_const"							<< endl
	<< " *   or "																			<< endl
	<< " *     shrink_weights_const * number_of_current_non_zero_values"					<< endl
	<< " *   or "																			<< endl
	<< " * 	number_of_current_non_zero_values - shrink_weights_const"						<< endl
<< endl
	<< " *  -shrink_weights_const f"
	<< " *   see documents for shrink_weights_method"										<< endl
<< endl
	<< " *  -use_delta_bar_delta [true]/[false]  "											<< endl
	<< " *   When true, we will use delta-bar-delta to calculate dynamic step sizes in SARSA"  << endl
<< endl
	<< " *  -use_idbd [true]/[false]  "														<< endl
	<< " *   When true, we will use iDBD to calculate dynamic step sizes in SARSA"			<< endl
<< endl
	<< " *  -end_episode_with_reward [true/false]"											<< endl
	<< " *   When true, current episode ends after each reward. This is useful in solving "	<< endl
	<< " *   some games. Default is false."													<< endl
<< endl
	<< " *  -epsilon_dim_start f"                                                           << endl
    << " * Starts diminishing epsilon after the give nratio of  episodes. -1"               << endl
    << " * (default) means never"                                                           << endl
<< endl
	<< " * Class-Agent Paramaters (also loaded from 'rl_params.txt')"						<< endl
	<< " *  -include_abs_positions [true]/[false]"											<< endl
	<< " *  -num_tilings n"																	<< endl
	<< " *  -num_rectangles n"																<< endl
	<< " *  -mem_size_multiplier n"															<< endl
	<< " *  -max_obj_velocity n"															<< endl
	<< " *   Maximum velocity (pixel/sec) of objects on screen. Default is 8"				<< endl
	<< " *  -max_num_detected_instaces n"													<< endl
	<< " *   Maximum number of instances that will be detected from each class."			<< endl
	
<< endl
<< endl
	<< " * Grid-Screen Agent Paramaters (also loaded from 'rl_params.txt')"					<< endl
	<< " *  -num_block_per_row n"															<< endl
	<< " *   How many blocks to have per row. Default is 10"								<< endl
<< endl
	<< " *  -num_block_per_col n"															<< endl
	<< " *   How many blocks to have per column. Default is 10"								<< endl
<< endl
	<< " *  -do_subtract_background [true/false]"											<< endl
	<< " *   When true (default), we will subtract the background before generating "		<< endl
	<< " *   the feature vector"															<< endl
	<< " *  -plot_gridscr_grids [true/false]"												<< endl
	<< " *   When true, we will plot the grid extracted for the screen by direc-screen agent."<< endl
    << endl
<< endl
	<< " * Class-Discovery Settings (usually loaded from 'class_disc_params.txt') :"		<< endl
	<< " *  -do_bg_detection [true]/[false]"												<< endl
	<< " *   When true, the player-agent will also do backgroudn detection."				<< endl
<< endl
	<< " *  -do_class_disc [true]/[false]"													<< endl
	<< " *   When true, the player-agent will also do class discovery."						<< endl
<< endl
	<< " *  -cls_disc_frames_num n"															<< endl
	<< " *   Number of frames to use for class discovery."									<< endl
<< endl
	<< " *  -plot_region_matrix_pre_merge [true]/[false]	"								<< endl
	<< " *   When true, we will plot the region matrix, befor mering the objects"			<< endl
<< endl
	<< " *  -plot_region_matrix_post_merge [true]/[false]"									<< endl
	<< " *   When true, we will plot the region matrix, after mering the objects"			<< endl
<< endl
	<< " *  -plot_pre_filter_classes [true]/[false]	"										<< endl
	<< " *   When true, we will plot the pre-filter discovered classes"						<< endl
<< endl
	<< " *  -plot_post_filter_classes [true]/[false]"										<< endl
	<< " *   When true, we will plot the post-filter discovered classes"					<< endl
<< endl
	<< " *  -max_perc_difference f"															<< endl
	<< " *   The maximum percentage of pixels of the two shapes that can be different,"		<< endl
	<< " *   and two shapes still be considered  similar. Default is 0.1"					<< endl
<< endl
	<< " *  -max_shape_area_dif f"															<< endl
	<< " *   How much two shape areas can differ ,and the distance between them still be"	<< endl
	<< " *   meaningful. Default is a factor of 1.2"										<< endl
<< endl
	<< " *  -min_on_frame_ratio f"															 << endl
	<< " *   Minimum ratio that a class needs to be on screen to be considered a valid class"<< endl
<< endl
	<< " *  -min_boundary_length n"															 << endl
	<< " *   For a class to be valid, either it has to expand at least i_min_boundary_length"<< endl 
	<< " *   on X or Y axis or the area it expands must be larger than min_boundary_length ^ 2"<< endl
<< endl
	<< " *  -max_num_classes n"																<< endl
	<< " *   Maximum acceptable number of classes that can be discovered"					<< endl
<< endl
<< endl
	<< " * Search-Agent Agent Paramaters (loaded from 'search_params.txt')"				<< endl
	<< " *  -search_method [fulltree/uct]"													<< endl
	<< " *   Determines the method to be used by the Search-Agent "						<< endl
<< endl
	<< " *  -sim_steps_per_node n"															<< endl
	<< " *   Number of steps we will run the simulation in each search-tree node"			<< endl
<< endl
	<< " *  -max_sim_steps_per_frame n"													<< endl
	<< " *   Maximum number of simulation steps allowed per frame"							<< endl
<< endl
	<< " *  -discount_factor f"															<< endl
	<< " *   The discount factor to help the tree-search prefer closer goals"				<< endl
<< endl
	<< " *  -uct_monte_carlo_steps n"														<< endl
	<< " *   Number of simulated Monte Carlo steps that will be run on each UCT iteration"	<< endl
<< endl
	<< " *  -uct_exploration_const f"														<< endl
	<< " *   Exploration Constant"															<< endl
<< endl
	<< " *  -uct_min_death_count n"														<< endl
	<< " *   Minimum number of simulations that should end up dead from a node, "			<< endl
	<< " *   before we mark that node as dead."											<< endl
<< endl
	<< " *  -uct_branch_value_method [average / max]"										<< endl
	<< " *   How the value of a branch is determined. 'average' takes the average value of "<< endl
	<< " *   the children, while 'max' takes the max value"								<< endl
<< endl
	<< " *  -uct_avg_reward_per_frame [true]/[false]"										<< endl
	<< " *   When true, uct will look at reward/frame (not just reward). This is to prevent"<< endl
	<< " *   biasing towards exploring already deeper sub-branches"						<< endl
<< endl
    << endl;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::saveConfig()
{
  // Do a quick scan of the internal settings to see if any have
  // changed.  If not, we don't need to save them at all.
  bool settingsChanged = false;
  for(unsigned int i = 0; i < myInternalSettings.size(); ++i)
  {
    if(myInternalSettings[i].value != myInternalSettings[i].initialValue)
    {
      settingsChanged = true;
      break;
    }
  }

  if(!settingsChanged)
    return;

  ofstream out(myOSystem->configFile().c_str());
  if(!out || !out.is_open())
  {
    cerr << "Error: Couldn't save settings file\n";
    return;
  }

  out << ";  Stella configuration file" << endl
      << ";" << endl
      << ";  Lines starting with ';' are comments and are ignored." << endl
      << ";  Spaces and tabs are ignored." << endl
      << ";" << endl
      << ";  Format MUST be as follows:" << endl
      << ";    command = value" << endl
      << ";" << endl
      << ";  Commmands are the same as those specified on the commandline," << endl
      << ";  without the '-' character." << endl
      << ";" << endl
      << ";  Values are the same as those allowed on the commandline." << endl
      << ";  Boolean values are specified as 1 (or true) and 0 (or false)" << endl
      << ";" << endl;

  // Write out each of the key and value pairs
  for(unsigned int i = 0; i < myInternalSettings.size(); ++i)
  {
    out << myInternalSettings[i].key << " = " <<
           myInternalSettings[i].value << endl;
  }

  out.close();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setInt(const string& key, const int value)
{
  ostringstream stream;
  stream << value;

  if(int idx = getInternalPos(key) != -1)
    setInternal(key, stream.str(), idx);
  else
    setExternal(key, stream.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setFloat(const string& key, const float value)
{
  ostringstream stream;
  stream << value;

  if(int idx = getInternalPos(key) != -1)
    setInternal(key, stream.str(), idx);
  else
    setExternal(key, stream.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setBool(const string& key, const bool value)
{
  ostringstream stream;
  stream << value;

  if(int idx = getInternalPos(key) != -1)
    setInternal(key, stream.str(), idx);
  else
    setExternal(key, stream.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setString(const string& key, const string& value)
{
  if(int idx = getInternalPos(key) != -1)
    setInternal(key, value, idx);
  else
    setExternal(key, value);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::getSize(const string& key, int& x, int& y) const
{
  string size = getString(key);
  replace(size.begin(), size.end(), 'x', ' ');
  istringstream buf(size);
  buf >> x;
  buf >> y;
}

bool Settings::hasString(const string& key) const {
  return getInternalPos(key) != -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Settings::getInt(const string& key, bool strict) const {
    // Try to find the named setting and answer its value
    int idx = -1;
    if((idx = getInternalPos(key)) != -1) {
        return (int) atoi(myInternalSettings[idx].value.c_str());
    } else { 
        if((idx = getExternalPos(key)) != -1) {
            return (int) atoi(myExternalSettings[idx].value.c_str());
        } else {
            if (strict) {
                cerr << "No value found for key: " << key << ". ";
                cerr << "Make sure all the settings files are loaded." << endl;
                exit(-1);
            } else {
                return -1;
            }
        }
    }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
float Settings::getFloat(const string& key, bool strict) const {
    // Try to find the named setting and answer its value
    int idx = -1;
    if((idx = getInternalPos(key)) != -1) {
        return (float) atof(myInternalSettings[idx].value.c_str());
    } else { 
        if((idx = getExternalPos(key)) != -1) {
            return (float) atof(myExternalSettings[idx].value.c_str());
        } else {
            if (strict) {
                cerr << "No value found for key: " << key << ". ";
                cerr << "Make sure all the settings files are loaded." << endl;
                exit(-1);
            } else {
                return -1.0;
            }
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Settings::getBool(const string& key, bool strict) const {
	// Try to find the named setting and answer its value
	int idx = -1;
	if((idx = getInternalPos(key)) != -1)
	{
		const string& value = myInternalSettings[idx].value;
		if(value == "1" || value == "true" || value == "True")
			return true;
		else if(value == "0" || value == "false" || value == "False")
			return false;
		else
			return false;
	} else if((idx = getExternalPos(key)) != -1) {
		const string& value = myExternalSettings[idx].value;
		if(value == "1" || value == "true")
			return true;
		else if(value == "0" || value == "false")
			return false;
		else
			return false;
	} else {
        if (strict) {
            cerr << "No value found for key: " << key << ". ";
            cerr << "Make sure all the settings files are loaded." << endl;
            exit(-1);
        } else {
            return false;
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string& Settings::getString(const string& key, bool strict) const {
    // Try to find the named setting and answer its value
    int idx = -1;
    if((idx = getInternalPos(key)) != -1) {
        return myInternalSettings[idx].value;
    } else if ((idx = getExternalPos(key)) != -1) {
        return myExternalSettings[idx].value;
    } else {
        if (strict) {
            cerr << "No value found for key: " << key << ". ";
            cerr << "Make sure all the settings files are loaded." << endl;
            exit(-1);
        } else {
            return EmptyString;
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setSize(const string& key, const int value1, const int value2)
{
  ostringstream buf;
  buf << value1 << "x" << value2;
  setString(key, buf.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Settings::getInternalPos(const string& key) const
{
  for(unsigned int i = 0; i < myInternalSettings.size(); ++i)
    if(myInternalSettings[i].key == key)
      return i;

  return -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Settings::getExternalPos(const string& key) const
{
  for(unsigned int i = 0; i < myExternalSettings.size(); ++i)
    if(myExternalSettings[i].key == key)
      return i;

  return -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Settings::setInternal(const string& key, const string& value,
                          int pos, bool useAsInitial)
{
  int idx = -1;

  if(pos != -1 && pos >= 0 && pos < (int)myInternalSettings.size() &&
     myInternalSettings[pos].key == key)
  {
    idx = pos;
  }
  else
  {
    for(unsigned int i = 0; i < myInternalSettings.size(); ++i)
    {
      if(myInternalSettings[i].key == key)
      {
        idx = i;
        break;
      }
    }
  }

  if(idx != -1)
  {
    myInternalSettings[idx].key   = key;
    myInternalSettings[idx].value = value;
    if(useAsInitial) myInternalSettings[idx].initialValue = value;

    /*cerr << "modify internal: key = " << key
         << ", value  = " << value
         << ", ivalue = " << myInternalSettings[idx].initialValue
         << " @ index = " << idx
         << endl;*/
  }
  else
  {
    Setting setting;
    setting.key   = key;
    setting.value = value;
    if(useAsInitial) setting.initialValue = value;

    myInternalSettings.push_back(setting);
    idx = myInternalSettings.size() - 1;

    /*cerr << "insert internal: key = " << key
         << ", value  = " << value
         << ", ivalue = " << setting.initialValue
         << " @ index = " << idx
         << endl;*/
  }

  return idx;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Settings::setExternal(const string& key, const string& value,
                          int pos, bool useAsInitial)
{
  int idx = -1;

  if(pos != -1 && pos >= 0 && pos < (int)myExternalSettings.size() &&
     myExternalSettings[pos].key == key)
  {
    idx = pos;
  }
  else
  {
    for(unsigned int i = 0; i < myExternalSettings.size(); ++i)
    {
      if(myExternalSettings[i].key == key)
      {
        idx = i;
        break;
      }
    }
  }

  if(idx != -1)
  {
    myExternalSettings[idx].key   = key;
    myExternalSettings[idx].value = value;
    if(useAsInitial) myExternalSettings[idx].initialValue = value;

    /*cerr << "modify external: key = " << key
         << ", value = " << value
         << " @ index = " << idx
         << endl;*/
  }
  else
  {
    Setting setting;
    setting.key   = key;
    setting.value = value;
    if(useAsInitial) setting.initialValue = value;

    myExternalSettings.push_back(setting);
    idx = myExternalSettings.size() - 1;

    /*cerr << "insert external: key = " << key
         << ", value = " << value
         << " @ index = " << idx
         << endl;*/
  }

  return idx;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Settings::Settings(const Settings&)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Settings& Settings::operator = (const Settings&)
{
  assert(false);

  return *this;
}
