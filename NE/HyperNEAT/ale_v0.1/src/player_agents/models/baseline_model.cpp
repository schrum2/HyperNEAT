#include "baseline_model.h"

void BaselineModel::update(IntArr* state, Action action, float reward, IntArr* nextState) {};
void BaselineModel::predict(IntArr* state, Action action, float* reward, IntArr* _nextState) {
  (*_nextState) = (*state);
};
