#ifndef __RECORDING_AGENT_HPP__
#define __RECORDING_AGENT_HPP__

#include "../common/Constants.h"
#include "PlayerAgent.hpp"
#include "../emucore/OSystem.hxx"

class RecordingAgent : public PlayerAgent {
public:
    RecordingAgent(OSystem * _osystem, RomSettings * _settings);
		
protected:
    int screen_width, screen_height;
    virtual Action act();
    ifstream trajFile;
    IntMatrix screen_matrix;
    string imgDir;
};

#endif 
