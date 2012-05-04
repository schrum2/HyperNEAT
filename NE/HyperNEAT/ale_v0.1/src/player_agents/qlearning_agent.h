/* *****************************************************************************
 *  qlearning_agent.h
 *
 * The implementation of the QLearning class, which uses QLearning to act.
 **************************************************************************** */

#ifndef QLEARNING_AGENT_H
#define QLEARNING_AGENT_H

#include "common_constants.h"
#include "player_agent.h"
#include <deque>

class QLearningAgent : public PlayerAgent {
public:
    QLearningAgent(GameSettings* _game_settings, OSystem* _osystem);
        
    /* *********************************************************************
       Returns a random action from the set of possible actions
       ******************************************************************** */
    virtual Action agent_step(const IntMatrix* screen_matrix, 
                              const IntVect* console_ram, int frame_number);

protected:
    float getQVal(const IntMatrix& screen, Action action);

protected:
    deque<pair<IntMatrix,Action> > keyq;

    map<pair<IntMatrix,Action>,float> qValues;

    float initial_val;

    ActionVect possible_actions;

    float alpha, gamma, epsilon;
};

#endif




