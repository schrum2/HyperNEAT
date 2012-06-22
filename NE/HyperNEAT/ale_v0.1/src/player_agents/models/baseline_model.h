#ifndef BASELINE_MODEL_H
#define BASELINE_MODEL_H

#include "model.h"
#include "../common_constants.h"

/***************************************************************
 Baseline model is the simplest model possible.
****************************************************************/
class BaselineModel : public Model {
 public:
  BaselineModel() {};
  ~BaselineModel() {};

  void predict(IntArr* state, Action action, float* reward, IntArr* nextState);
  // Update the model with an experience tuple
  void update(IntArr* state, Action action, float reward, IntArr* nextState);
  BaselineModel* clone() const { return new BaselineModel(*this); };
  void declone() { delete this; };
};

#endif
