#include "model_agent.h"
#include "SDL/SDL.h"


ModelAgent::ModelAgent(GameSettings* _game_settings, OSystem* _osystem):
  BassAgent(_game_settings, _osystem),
  //  model(i_num_block_per_row,i_num_block_per_col),
  lastState(0, i_blocks_bits_length),
  predictedNextState(0, i_blocks_bits_length),
  i_next_act_frame(0), step_num(0),
  uct_planner(_osystem->settings(), _game_settings->pv_possible_actions),
  width(i_num_block_per_row), height(i_num_block_per_col),
  totalAcc(0.0), pred_cnt(0), grounded(false)
{
  e_curr_action = UNDEFINED;
  
  Settings& settings = p_osystem->settings();
  string model_type = settings.getString("model_type",true);
  cout << "Model type: " << model_type << endl;
  if (model_type == "baseline")
    model = new BaselineModel();
  else if (model_type == "freeway")
    model = new FreewayModel(i_num_block_per_row, i_num_block_per_col);
  else if (model_type == "dbn")
    model = new BasicDBN(i_num_block_per_row, i_num_block_per_col);
  else if (model_type == "logistic")
    model = new Logistic(i_num_block_per_row, i_num_block_per_col, _game_settings->pv_possible_actions, p_osystem);
  else {
    cerr << "Invalid model type specified." << endl;
    exit(-1);
  }

  trainingEnd = settings.getInt("end_training_time",true);
  eval_frames = settings.getInt("eval_frames",true);
  expr_filename = settings.getString("expr_filename",true);
}

ModelAgent::~ModelAgent() {
  delete model;
  uct_planner.clear();
}

Action ModelAgent::agent_step(IntArr* curr_state) {
  if (i_frame_counter >= i_next_act_frame) {
    cout << "Frame " << i_frame_counter << endl;
    // Run a new simulation to find the next action
    //i_next_act_frame = i_frame_counter + i_sim_steps_per_node;
    // cout << "Agent_Step Frame: " << i_frame_counter << ", ";
    // if (uct.is_built) {
    //   // Re-use the old tree
    //   uct.move_to_best_sub_branch();
    //   //assert(uct.get_root_frame_number() == i_frame_counter);
    //   uct.update_tree(*model, *curr_state);
    //   cout << "Tree Updated: ";
    // } else {
    //   // Build a new Search-Tree
    //   uct.clear(); 
    //   uct.build(*model, *curr_state);
    //   cout << "Tree Re-Constructed: ";
    // }
    // lastAction = uct.get_best_action();
    // if (rand() / (float) RAND_MAX < .3)
    //   lastAction = PLAYER_A_UP;//choice(p_game_settings->pv_possible_actions);//RANDOM;//PLAYER_A_NOOP;
    // else
    lastAction = choice(p_game_settings->pv_possible_actions);
    float curr_reward = p_game_settings->get_reward(pm_curr_screen_matrix, pv_curr_console_ram);

    // Batch train if needed
    if (step_num == trainingEnd) {
      if (p_osystem->settings().getString("model_type",true) == "logistic")
        ((Logistic*)model)->runOptimization();
      // Reset the game
      e_episode_status = RESTART_DELAY;
      i_restart_delay_counter=p_game_settings->i_delay_after_restart;
      lastAction = RESET;
    } 

    // Evaluate Model's Predictions
    if (step_num > trainingEnd + 1) {
      if (step_num == trainingEnd + 2) {// Init predicted state correctly
        if (b_display_screen)
          runModel(&lastState);
        predictedNextState = lastState;
      }

      float pred_reward; // Ground predictions or not?
      if (grounded) model->predict(&lastState,          lastAction, &pred_reward, &predictedNextState);
      else          model->predict(&predictedNextState, lastAction, &pred_reward, &predictedNextState);

      BlockGrid bg_actual(curr_state,width,height,8);
      BlockGrid bg_predicted(&predictedNextState,width,height,8);
      float predictionAccuracy = bg_actual.getSimilarity(&bg_predicted);
      totalAcc += predictionAccuracy;
      pred_cnt++;
    }

    // Report Model's Performance
    if (step_num >= trainingEnd + eval_frames + 1) {
      printf("Training time: %d steps. Avg accuracy of %f over %d steps.\n",trainingEnd,totalAcc/pred_cnt,pred_cnt);
      if (!expr_filename.empty()) {
        ofstream myfile;
        myfile.open(expr_filename.c_str(), ios::out | ios::app);
        myfile << "Training time: " << trainingEnd << " steps. Avg accuracy: "
               << (totalAcc/pred_cnt) << " Eval Steps: " << pred_cnt << endl;
        myfile.close();
      }
      exit(0);
    }

    model->update(&lastState, lastAction, curr_reward, curr_state);
    lastState = (*curr_state);
    if (!p_osystem->settings().getString("load_model",true).empty()) {
      runModel(&lastState);
      exit(-1);
    }
    step_num++;
    // cout << " Root Value = " << search_tree.get_root_value();  
    // cout << " - Deepest Node Frame: " 
    //      << search_tree.i_deepest_node_frame_num << endl;

    // deal with the bloody bug, where the screen doesnt get updated
    // after restoring the state for one turn. This *hack* allows 
    // basically skips exporting teh screen for one turn
    // i_skip_export_on_frame = i_frame_counter + 1;
  }
  return lastAction;
}



// Test method which continuously runs and visualizes model output.
void ModelAgent::runModel(IntArr* pv_feature_vec) {
  cout << "Running away in the model..." << endl;
  IntArr currState(0, i_blocks_bits_length);
  currState = (*pv_feature_vec);
  BassAgent::plot_extracted_grid(&currState);
  BassAgent::display_bass_screen();
  IntArr nxtState(0, i_blocks_bits_length);
  int cnt = 0;
  while(1) {
    cnt++;
    cout << "loop" << cnt <<  endl;
    Action a = choice <Action> (p_game_settings->pv_possible_actions);

    // Choose action from keypress
    bool haveAction = false;
    while (!haveAction) {
      SDL_PumpEvents();
      Uint8 * keymap = SDL_GetKeyState(0);
      if (keymap[SDLK_q]) {
        return;
        
        // Trips
      } else if (keymap[SDLK_UP] && keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_UPRIGHTFIRE;
      } else if (keymap[SDLK_UP] && keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_UPLEFTFIRE;
      } else if (keymap[SDLK_DOWN] && keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_DOWNRIGHTFIRE;
      } else if (keymap[SDLK_DOWN] && keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_DOWNLEFTFIRE;

        // Doubs
      } else if (keymap[SDLK_UP] && keymap[SDLK_LEFT]) {
        haveAction = true;
        a = PLAYER_A_UPLEFT;
      } else if (keymap[SDLK_UP] && keymap[SDLK_RIGHT]) {
        haveAction = true;
        a = PLAYER_A_UPRIGHT;
      } else if (keymap[SDLK_DOWN] && keymap[SDLK_LEFT]) {
        haveAction = true;
        a = PLAYER_A_DOWNLEFT;
      } else if (keymap[SDLK_DOWN] && keymap[SDLK_RIGHT]) {
        haveAction = true;
        a = PLAYER_A_DOWNRIGHT;
      } else if (keymap[SDLK_UP] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_UPFIRE;
      } else if (keymap[SDLK_DOWN] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_DOWNFIRE;
      } else if (keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_LEFTFIRE;
      } else if (keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_RIGHTFIRE;

        // Singles
      } else if (keymap[SDLK_SPACE]) {
        haveAction = true;
        a = PLAYER_A_FIRE;
      } else if (keymap[SDLK_RETURN]) {
        haveAction = true;
        a = PLAYER_A_NOOP;
      } else if (keymap[SDLK_LEFT]) {
        haveAction = true;
        a = PLAYER_A_LEFT;
      } else if (keymap[SDLK_RIGHT]) {
        haveAction = true;
        a = PLAYER_A_RIGHT;
      } else if (keymap[SDLK_UP]) {
        haveAction = true;
        a = PLAYER_A_UP;
      } else if (keymap[SDLK_DOWN]) {
        haveAction = true;
        a = PLAYER_A_DOWN;
      } 
      SDL_Delay(5); // Set amount of sleep time
    }

    float reward;
    model->predict(&currState, a, &reward, &nxtState);
    model->update(&currState,a,reward,&nxtState);
    BassAgent::plot_extracted_grid(&nxtState);
    BassAgent::display_bass_screen();

    currState = nxtState;
  }
}

