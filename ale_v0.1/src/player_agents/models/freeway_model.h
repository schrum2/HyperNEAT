#ifndef FREEWAY_MODEL_H
#define FREEWAY_MODEL_H

#include "model.h"
#include "../common_constants.h"
#include "block_grid.h"
#include <deque>
#include <map>

class FreewayModel : public Model {
 public:
  FreewayModel(int width, int height); // Dimensions of the bass array
  ~FreewayModel();

  // Given state and action, the model will predict reward and next state.
  void predict(IntArr* state, Action action, float* reward, IntArr* nextState);
  // Update the model with an experience tuple
  void update(IntArr* state, Action action, float reward, IntArr* nextState);
  FreewayModel* clone() const { return new FreewayModel(*this); };
  void declone() { delete this; };

 protected:
  void getCarRules(int laneNum, int colorIndx, IntArr* pv_car_rules);
  void predictCars(BlockGrid* grid, BlockGrid* nextState);

  void predictChicken(BlockGrid* grid, Action action, BlockGrid* nextState);
  void predictCollisions(BlockGrid* grid, Action action, BlockGrid* nextState);
  bool hasHistory(int x, int y, int colorIndx, int historyLen);

  int width;
  int height;
  deque<BlockGrid> stateHistory;
  deque<Action> actionHistory;
  IntVect pv_tmp_indexes;
  IntArr pv_tmp_color_bits;
  IntArr pv_car_rules;
  int chickenPos;
  int chickenCtr;
  static const int numColors = 8;
  static const int maxHistoryLen = 15;
  Action lastChickenAct;
  int timeSinceHit;
  bool crossed;
  int restartTimer;
};

#endif
