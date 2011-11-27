#include "freeway_model.h"

FreewayModel::FreewayModel(int width, int height):stateHistory(), actionHistory(),
                                                  pv_tmp_indexes(), pv_tmp_color_bits(0,numColors),
                                                  pv_car_rules(0,3) {
  this->width = width;
  this->height = height;
  chickenCtr = 0;
  lastChickenAct = PLAYER_A_UP;
  timeSinceHit = -1;
  crossed = false;
  restartTimer = -1;
}

void FreewayModel::getCarRules(int laneNum, int colorIndx, IntArr* pv_car_rules) {
  if (((laneNum == 2 || laneNum == 3) && colorIndx == BLUE) ||
      ((laneNum == 17) && colorIndx == CYAN)) {
    (*pv_car_rules)[0] = 4; // s2d
    (*pv_car_rules)[1] = 9; // d2s
    (*pv_car_rules)[2] = 12;// emergence time
  } else if (((laneNum == 4 || laneNum == 5) && colorIndx == YELLOW) ||
             ((laneNum == 15 || laneNum == 16) && colorIndx == GREEN)) {
    (*pv_car_rules)[0] = 3;
    (*pv_car_rules)[1] = 7;
    (*pv_car_rules)[2] = 10;
  } else if ((laneNum == 6 && colorIndx == BLUE) ||
             (laneNum == 14 && colorIndx == RED)) {
    (*pv_car_rules)[0] = 3;
    (*pv_car_rules)[1] = 5;
    (*pv_car_rules)[2] = 8;
  } else if (((laneNum == 7 || laneNum == 8) && colorIndx == CYAN) ||
             ((laneNum == 12 || laneNum == 13) && colorIndx == GREEN)) {
    (*pv_car_rules)[0] = 2;
    (*pv_car_rules)[1] = 3;
    (*pv_car_rules)[2] = 5;
  } else if ((laneNum == 9 && colorIndx == BLUE) ||
             ((laneNum == 10 || laneNum == 11) && colorIndx == RED)) {
    (*pv_car_rules)[0] = 1;
    (*pv_car_rules)[1] = 2;
    (*pv_car_rules)[2] = 2;
  } else {
    cerr << "Invalid lane and color indx for car data: Lane:" << laneNum << " colorIndx: " << colorIndx << endl;
    exit(-1);
  }
}

FreewayModel::~FreewayModel() {}

void FreewayModel::predict(IntArr* state, Action action, float* reward, IntArr* _nextState) {
  BlockGrid grid = BlockGrid(state, width, height, numColors);
  (*_nextState) = (*state);
  BlockGrid nextState = BlockGrid(_nextState, width, height, numColors);

  if (crossed)
    *reward = 1.0;
  else 
    *reward = 0.0;

  predictCars(&grid, &nextState);
  predictCollisions(&grid, action, &nextState);

  if (restartTimer >= 0)
    restartTimer++;
  else if (restartTimer > 9)
    restartTimer = -1;

  if (timeSinceHit >= 0 && timeSinceHit < 6)
    action = PLAYER_A_DOWN;
  else if (timeSinceHit >= 0 && timeSinceHit < 14)
    action = PLAYER_A_NOOP;
  if (restartTimer >= 0 && restartTimer < 8)
    action = PLAYER_A_NOOP;

  int oldNumWhite = 0;
  if (!stateHistory.empty()) {
    stateHistory[0].getIndexes(WHITE, &pv_tmp_indexes);
    oldNumWhite = pv_tmp_indexes.size();
  }
  grid.getIndexes(WHITE,&pv_tmp_indexes);
  int numWhite = pv_tmp_indexes.size();

  if (!stateHistory.empty() && numWhite != oldNumWhite) {
    chickenCtr = 0;
    lastChickenAct = actionHistory[0];
  }
  if (action == PLAYER_A_UP)
    chickenCtr++;
  else if (action == PLAYER_A_DOWN)
    chickenCtr--;
  if (numWhite == 1 && pv_tmp_indexes[1] == 12 && action == PLAYER_A_DOWN) {
    chickenCtr = 0;
    lastChickenAct = PLAYER_A_UP;
  }

  predictChicken(&grid, action, &nextState);

  (*_nextState) = nextState.grid;
}

void FreewayModel::update(IntArr* state, Action action, float reward, IntArr* nextState) {
  BlockGrid grid = BlockGrid(state,width,height,numColors);
  stateHistory.push_front(grid);
  actionHistory.push_front(action);
  while (stateHistory.size() > maxHistoryLen) {
    stateHistory.pop_back();
    actionHistory.pop_back();
  }
}

void FreewayModel::predictCars(BlockGrid* grid, BlockGrid* nextState) {
  for (int y = 2; y < height; y++) {
    for (int x = 0; x < width; x++) {
      grid->getColors(x,y,&pv_tmp_color_bits);
      for (int colorIndx = 0; colorIndx < numColors; colorIndx++) {
        if (colorIndx == WHITE || colorIndx == BLACK || pv_tmp_color_bits[colorIndx] == 0)
          continue;
        getCarRules(y,colorIndx,&pv_car_rules);
        if (x == 0) {
          if (y < 10) { // Dissapearance
            int d2s = pv_car_rules[1];
            int s2d = pv_car_rules[0];
            if (hasHistory(x,y,colorIndx,d2s+s2d) && !grid->hasColor(x+1,y,colorIndx)) {
              nextState->removeColor(x,y,colorIndx);
              nextState->addColor(15,y,colorIndx);
              nextState->addColor(15,y,BLACK);
            }
          } else { // Emergence
            int eTime = pv_car_rules[2];
            if (hasHistory(x,y,colorIndx,eTime))
              nextState->addColor(x+1,y,colorIndx);
          }
        } else if (x == 15) {
          if (y < 10) { // Emergence
            int eTime = pv_car_rules[2];
            if (hasHistory(x,y,colorIndx,eTime))
              nextState->addColor(x-1,y,colorIndx);
          } else { // Dissapearance
            int d2s = pv_car_rules[1];
            int s2d = pv_car_rules[0];
            if (hasHistory(x,y,colorIndx,d2s+s2d) && !grid->hasColor(x-1,y,colorIndx)) {
              nextState->removeColor(x,y,colorIndx);
              nextState->addColor(0,y,colorIndx);
              nextState->addColor(0,y,BLACK);
            }
          }
        }

        if (grid->hasColor(x+1,y,colorIndx)) {
          int d2s = pv_car_rules[1];
          if (y >= 10 && hasHistory(x,y,colorIndx,d2s) && hasHistory(x+1,y,colorIndx,d2s)) {
            nextState->removeColor(x,y,colorIndx);
            nextState->removeColor(x,y,BLACK);
          }
        } else if (grid->hasColor(x-1,y,colorIndx)) {
          int d2s = pv_car_rules[1];
          if (y < 10 && hasHistory(x,y,colorIndx,d2s) && hasHistory(x-1,y,colorIndx,d2s)) {
            nextState->removeColor(x,y,colorIndx);
            nextState->removeColor(x,y,BLACK);
          }
        } else {
          int s2d = pv_car_rules[0];
          if (hasHistory(x,y,colorIndx,s2d)) {
            if (y < 10) {
              nextState->addColor(x-1,y,colorIndx);
              nextState->addColor(x-1,y,BLACK);
            } else {
              nextState->addColor(x+1,y,colorIndx);
              nextState->addColor(x+1,y,BLACK);
            }
          }
        }
      }
    }
  }
}

bool FreewayModel::hasHistory(int x, int y, int colorIndx, int historyLen) {
  if (stateHistory.size() < historyLen) 
    return false;
  
  for (int i = 0; i < historyLen-1; i++) 
    if (!stateHistory[i].hasColor(x,y,colorIndx)) 
      return false;

  return true;
}

void FreewayModel::predictChicken(BlockGrid* grid, Action action, BlockGrid* nextState) {
  crossed = false;
  pv_tmp_indexes.clear();
  grid->getIndexes(WHITE,&pv_tmp_indexes);
  // Has chicken has crossed the road?
  if (pv_tmp_indexes.size() >= 2 && pv_tmp_indexes[0] == 4 && pv_tmp_indexes[1] == 1 && action == PLAYER_A_UP && !stateHistory.empty()) {
    IntVect tmp_vect;
    stateHistory[0].getIndexes(WHITE,&tmp_vect);
    if ((tmp_vect[0] == 4 && tmp_vect[1] == 2) ||
        (tmp_vect[0] == 5 && tmp_vect[1] == 1)) {
      grid->getIndexes(WHITE,&pv_tmp_indexes);
      for (int i = 0; i < pv_tmp_indexes.size()/2; i++)
        nextState->removeColor(pv_tmp_indexes[2*i],pv_tmp_indexes[2*i+1],WHITE);
      nextState->addColor(4,19,WHITE);
      crossed = true;
      lastChickenAct = PLAYER_A_UP;
      timeSinceHit = -1;
      restartTimer = 0;
      return;
    }
  }

  // Erase all pairs not in col 4
  for (int i = 0; i < pv_tmp_indexes.size()/2; i++) {
    if (pv_tmp_indexes[2*i] != 4) {
      pv_tmp_indexes.erase(pv_tmp_indexes.begin()+2*i);
      pv_tmp_indexes.erase(pv_tmp_indexes.begin()+2*i);
    }
  }

  // Single to double chicken movement
  if (pv_tmp_indexes.size() == 2) {
    int x = pv_tmp_indexes[0];
    int y = pv_tmp_indexes[1];
    if (action == PLAYER_A_UP)
      nextState->addColor(x,y-1,WHITE);
    else if (action == PLAYER_A_DOWN)
      nextState->addColor(x,y+1,WHITE);
  } else if (pv_tmp_indexes.size() == 4) { // Double to single chicken movement
    if (action == PLAYER_A_UP) {
        int x = pv_tmp_indexes[2];
        int y = pv_tmp_indexes[3];
        nextState->removeColor(x,y,WHITE);
    } else if (action == PLAYER_A_DOWN) {
        int x = pv_tmp_indexes[0];
        int y = pv_tmp_indexes[1];
        nextState->removeColor(x,y,WHITE);
    }
  } else {
    cerr << "Unexpected number of chicken squares:" << pv_tmp_indexes.size() << endl;
  }
}

void FreewayModel::predictCollisions(BlockGrid* grid, Action action, BlockGrid* nextState) {
  grid->getIndexes(WHITE,&pv_tmp_indexes);
  for (int i = 0; i < pv_tmp_indexes.size()/2; i++) {
    int x = pv_tmp_indexes[i*2];
    if (x != 4) continue;
    int y = pv_tmp_indexes[i*2+1];
    grid->getColors(x,y,&pv_tmp_color_bits);
    // Check if we overlap with a car and tires
    if (pv_tmp_color_bits.sum() >= 3 && pv_tmp_color_bits[BLACK])
      timeSinceHit = 0;
    if (timeSinceHit >= 0)
      timeSinceHit++;
    if (timeSinceHit >= 14)
      timeSinceHit = -1;
  }
}









