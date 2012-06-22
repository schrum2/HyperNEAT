/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  background_detector.h
 *
 *  The implementation of the BackgroundDetector class, which is responsible for
 *  detecting the background from the given screens
 **************************************************************************** */

#ifndef BACKGROUND_DETECTOR_H
#define BACKGROUND_DETECTOR_H

#include "common_constants.h"
#include "../emucore/OSystem.hxx"
#include "game_settings.h"
#define NUM_COLORS 256

class BackgroundDetector  {
  /* *************************************************************************
     Responsible for detecting the background from the given screens
     In the current implementation we use a simple frequency method, i.e.
     we choose the most frequent color for each pixel as the background
        
     Instance variabls:
     - i_frames_num			Number of frames to use for background detection
     - i_frames_counter		Counts the number of frames we have seen
     - v_color_counts		In this vector, each pixel has NUM_COLORS
     digits assigned to it, each of which counts the 
     number of frames that had the particular color 
     in this pixel
     - i_screen_width		Width of the screen
     - i_screen_height		Height of the screen
     ************************************************************************* */

 public:
  BackgroundDetector(GameSettings* _game_settings, OSystem* _osystem);
  virtual ~BackgroundDetector();

  /* *********************************************************************
     Recieves a new screen, and updates the color counts
     ******************************************************************** */
  virtual void get_new_screen(const IntMatrix* pv_screen);

  /* *********************************************************************
     Extracts the background matrix from the color-counts and exports it
     as .txt and .png files
     ******************************************************************** */
  virtual void extract_and_save_background();
		
  /* *********************************************************************
     Returns true when the background is extracted and saved
     ******************************************************************** */
  bool is_bg_extraction_complete() {
    return (i_frames_counter > i_frames_num);
  }
		
 protected:
  /* *********************************************************************
     given a pixel coordination (i, j), returns the color index with 
     highest value in v_color_counts
     ******************************************************************** */
  int get_most_frequent_color_ind(int i, int j);
		
  OSystem* p_osystem;
  int i_frames_num;
  int i_frames_counter; 
  IntVect v_color_counts;
  int i_screen_width;
  int i_screen_height;
  string romfile;
};

#endif
