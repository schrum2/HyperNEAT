#include "RecordingAgent.hpp"
#include "random_tools.h"
#include <boost/lexical_cast.hpp>
#include <assert.h>

RecordingAgent::RecordingAgent(OSystem* _osystem, RomSettings* _settings) : 
    PlayerAgent(_osystem, _settings) {
    Settings& settings = p_osystem->settings();
    string f = settings.getString("trajectory_file", false);
    assert(!f.empty());
    imgDir = settings.getString("img_dir", false);
    assert(!imgDir.empty());

    trajFile.open(f.c_str(), std::ifstream::in);
    assert(trajFile.good());

    screen_width = _osystem->console().mediaSource().width();
    screen_height = _osystem->console().mediaSource().height();    
    // Initialize our screen matrix
    for (int i=0; i<screen_height; ++i) { 
        IntVect row;
        for (int j=0; j<screen_width; ++j)
            row.push_back(-1);
        screen_matrix.push_back(row);
    }
}

Action RecordingAgent::act() {
    static int frame = 0;
    uInt8* pi_curr_frame_buffer = p_osystem->console().mediaSource().currentFrameBuffer();
    int ind_i, ind_j;
    for (int i = 0; i < screen_width * screen_height; i++) {
        uInt8 v = pi_curr_frame_buffer[i];
        ind_i = i / screen_width;
        ind_j = i - (ind_i * screen_width);
        screen_matrix[ind_i][ind_j] = v;
    }
    string fname = imgDir + boost::lexical_cast<string>(frame) + ".png";
    p_osystem->p_export_screen->save_png(&screen_matrix, fname);
    frame++;

    assert(trajFile.good());
    string line;
    std::getline(trajFile, line);
    Action a = (Action) boost::lexical_cast<int>(line);
    return a;
}

