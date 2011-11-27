/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  background_detector.cpp
 *
 *  The implementation of the BackgroundDetector class, which is responsible for
 *  detecting the background from the given screens
 **************************************************************************** */

#include "background_detector.h"
#include "game_controller.h"
#include "export_tools.h"


BackgroundDetector::BackgroundDetector(GameSettings* _game_settings, OSystem* _osystem) {
  p_osystem = _osystem;
  i_frames_num = p_osystem->settings().getInt("bg_detect_frames_num", true);
  i_frames_counter = 0;
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
  v_color_counts.clear();
  for (int i = 0; i < i_screen_height * i_screen_width * NUM_COLORS; i++) {
    v_color_counts.push_back(0);
  }
  romfile = p_osystem->settings().getString("rom_file",true);
  romfile = romfile.substr(romfile.find_last_of("/\\")+1);
}


BackgroundDetector::~BackgroundDetector() {
  // nothing to do yet
}

/* *********************************************************************
   Recieves a new screen, and updates the color-counts
   ******************************************************************** */
void BackgroundDetector::get_new_screen(const IntMatrix* pv_screen) {
  if (i_frames_counter > i_frames_num) {
    return; // we've already extracted the background
  }
  int pixel_ind = 0;	// location for the current pixel in v_color_counts
  for (int i = 0; i < i_screen_height; i++) {
    for (int j = 0; j < i_screen_width; j++) {
      int color_ind = (*pv_screen)[i][j];
      assert(color_ind < NUM_COLORS);
      v_color_counts[pixel_ind + color_ind] += 1;
      pixel_ind += NUM_COLORS;
    }
  }
  i_frames_counter++;
  if (i_frames_counter == i_frames_num) {
    extract_and_save_background();
  }
}

/* *********************************************************************
   Extracts the background matrix from the color-counts and exports it
   as .txt and .png files
   ******************************************************************** */
void BackgroundDetector::extract_and_save_background() {
  // 1 - Extract the background matrix
  IntMatrix* pm_background = new IntMatrix;
  for (int i = 0; i < i_screen_height; i++) {
    IntVect row;
    for (int j = 0; j < i_screen_width; j++) {
      int most_frq_ind = get_most_frequent_color_ind(i, j);
      row.push_back(most_frq_ind);
    }
    pm_background->push_back(row);
  }
	
  // 2- Export the background matrix;
  export_matrix(pm_background, "backgrounds/" + romfile);
  p_osystem->p_export_screen->export_any_matrix(pm_background, "backgrounds/" + romfile + ".png");
	
  delete pm_background;
};


/* *********************************************************************
   given a pixel coordination (i, j), returns the color index with 
   highest value in v_color_counts
   ******************************************************************** */
int BackgroundDetector::get_most_frequent_color_ind(int i, int j) {
  int pixel_num = (i * i_screen_width) + j;
  int pixel_ind = pixel_num * NUM_COLORS;
  int highest_ind = 0;
  int highest_val = v_color_counts[pixel_ind];
  for (int i = 1; i < NUM_COLORS; i++ ) {
    int val = v_color_counts[pixel_ind + i];
    if (val > highest_val) {
      highest_val = val;
      highest_ind = i;
    }
  }
  return highest_ind;
}
