#ifndef MODEL_H
#define MODEL_H

#include "../common_constants.h"

/********************************************************************************
 Generic model class
********************************************************************************/
class Model {
 public:
  virtual void predict(IntArr* state, Action action, float* reward, IntArr* nextState) = 0;
  virtual void update(IntArr* state, Action action, float reward, IntArr* nextState) = 0;
  virtual Model* clone() const = 0;
  virtual void declone() = 0;
};

#endif
