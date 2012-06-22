#include "qlearning_agent.h"
#include "../common/random_tools.h"
#include "../emucore/Serializer.hxx"
#include "../emucore/Deserializer.hxx"
#include "../emucore/m6502/src/System.hxx"
#include <sstream>
#include <boost/functional/hash.hpp> // Used to define a hash function


QLearningAgent::QLearningAgent(GameSettings* _game_settings, OSystem* _osystem) : 
    PlayerAgent(_game_settings, _osystem), initial_val(1.0), alpha(1.0), gamma(.95), epsilon(0) {
    possible_actions = *(_game_settings->pv_possible_actions);
}

float QLearningAgent::getQVal(const IntMatrix& screen, Action action) {
    pair<IntMatrix,Action> sa (screen,action);
    if (qValues.find(sa) == qValues.end())
        return initial_val;
    else
        return qValues[sa];
}

Action QLearningAgent::agent_step(const IntMatrix* screen_matrix, 
                                  const IntVect* console_ram, 
                                  int frame_number) {
    Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram, frame_number);

    if (special_action != UNDEFINED)
        return special_action;  // We are resetting or in a delay

    Action action;// = choice <Action> (p_game_settings->pv_possible_actions);

    // Action selection
    if (rand()/(float)RAND_MAX < epsilon) {
        action = choice <Action> (p_game_settings->pv_possible_actions);
    } else {
        float highestVal = -numeric_limits<float>::max();
        vector<int> highestInds;
        for (int i = 0; i < possible_actions.size(); i++) {
            float val = getQVal(*screen_matrix,possible_actions[i]);
            if (val > highestVal) {
                highestVal = val;
                highestInds.clear();
                highestInds.push_back(i);
            } else if (fabs(highestVal - val) < .000001) {
                highestInds.push_back(i);
            }
        }
        action = possible_actions[choice<int>(&highestInds)];
    }


    if (!keyq.empty()) {
        // Q-Learning update
        pair<IntMatrix,Action> old_key = keyq.front();
        float oldQVal = qValues.find(old_key) == qValues.end() ? initial_val : qValues[old_key];
        float maxQVal = -numeric_limits<float>::max();
        for (int i=0; i<possible_actions.size(); ++i) {
            pair<IntMatrix,Action> sa (old_key.first,possible_actions[i]);
            if (qValues.find(sa) == qValues.end()) {
                if (initial_val > maxQVal)
                    maxQVal = initial_val;
                continue;
            } else if (qValues[sa] > maxQVal)
                maxQVal = qValues[sa];
        }
        qValues[old_key] = oldQVal + alpha * (f_curr_reward + gamma * maxQVal - oldQVal);
        alpha *= .99999;
    }

    pair<IntMatrix,Action> key (*screen_matrix, action);
    keyq.push_back(key);

    while (keyq.size() > 2) {
        keyq.pop_front();
    }

    return action;
}
