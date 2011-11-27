#ifndef MODEL_AGENT_H
#define MODEL_AGENT_H

#include "common_constants.h"
#include "bass_agent.h"
#include "models/baseline_model.h"
#include "models/freeway_model.h"
#include "models/basic_dbn.h"
#include "models/logistic.h"
#include "models/model.h"
#include "planners/uct.h"
#include "random_tools.h"

class ModelAgent : public BassAgent {
 public:
  ModelAgent(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~ModelAgent();

 protected:
  Action agent_step(IntArr* curr_state);
  void runModel(IntArr* pv_feature_vec);
  void predict(BlockGrid& state, Action action, float* reward, BlockGrid& nextState);

  Model* model;

  IntArr lastState;
  IntArr predictedNextState;
  Action lastAction;

  Action e_curr_action;	        // The action we are curently taking
  int i_next_act_frame;	        // The next frame where we need to pick an action
  int step_num;                 // Internal step counter
  UCT uct_planner;             	// The uct based planner

  int width, height;
  int trainingEnd, eval_frames; // Experimental params
  float totalAcc;
  int pred_cnt;
  bool grounded;                // Are next state predictions grounded
  string expr_filename;
};

#endif
