/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  fifo_controller.cpp
 *
 *  The implementation of the FIFOController class, which is a subclass of 
 * GameConroller, and is resposible for sending the Screens/RAM content to 
 * whatever external program we are using through FIFO pipes, and apply the 
 * actions that are sent back
 **************************************************************************** */

#include <sstream>
#include "fifo_controller.h"
#include "../common/export_tools.h"

/* *********************************************************************
    Constructor
 ******************************************************************** */ 
FIFOController::FIFOController(OSystem* _osystem) : 
  GameController(_osystem) {
  p_fout = fopen("ale_fifo_out", "w");
  p_fin = fopen("ale_fifo_in", "r");
  if (p_fout == NULL || p_fin == NULL) {
    cerr << "A.L.E expects two FIFO pipes to exist:\n";
    cerr << "\t ale_fifo_out (A.L.E will send its output here)\n";
    cerr << "\t ale_fifo_in (A.L.E will get its input from here)\n";
    exit(-1);
  }

  b_display_screen = p_osystem->settings().getBool("display_screen", true);
  b_use_bass = p_osystem->settings().getBool("use_bass",true);
		
  // Send the width and height of the screen through the pipe
  char out_buffer [50];
  cout << "i_screen_width = " << i_screen_width << " - i_screen_height =" <<   i_screen_height << endl;
  sprintf (out_buffer, "%d-%d\n", i_screen_width, i_screen_height);
  fputs(out_buffer, p_fout);
  fflush (p_fout);
  // Get confirmation that the values were sent
  char in_buffer [50];
  cerr<< "A.L.E: waiting for a reply ..." << endl;
  fgets (in_buffer, 50, p_fin);
  char * token = strtok (in_buffer,",\n");
  b_send_screen_matrix = atoi(token);
  token = strtok (NULL,",\n");
  b_send_console_ram = atoi(token);
  token = strtok (NULL,",\n");
  i_skip_frames_num = atoi(token);
  i_skip_frames_counter = i_skip_frames_num;
  cerr << "A.L.E: send_screen_matrix is: " << b_send_screen_matrix << endl;
  cerr << "A.L.E: send_console_ram is: " << b_send_console_ram << endl;
  cerr << "A.L.E: i_skip_frames_num is: " << i_skip_frames_num	<< endl;
  // Initialize our copy of frame_buffer 
  pi_old_frame_buffer = new int [i_screen_width * i_screen_height];
  for (int i = 0; i < i_screen_width * i_screen_height; i++) {
    pi_old_frame_buffer[i] = -1;
  }
  i_frame_number = 0;
  pm_screen_matrix = NULL;
  if (b_display_screen || b_use_bass) {
    pm_screen_matrix = new IntMatrix;
    assert(i_screen_height > 0);
    assert(i_screen_width > 0);
    for (int i = 0; i < i_screen_height; i++) {
      IntVect row;
      for (int j = 0; j < i_screen_width; j++) {
        row.push_back(-1);
      }
      pm_screen_matrix->push_back(row);
    }
  }

  // Initialize bass variables
  if (b_use_bass) {
    i_num_block_per_row = p_osystem->settings().getInt("num_block_per_row", true);
    i_num_block_per_col = p_osystem->settings().getInt("num_block_per_col", true);
    i_block_height = i_screen_height / i_num_block_per_row;
    i_block_width = i_screen_width / i_num_block_per_col;
    i_blocks_bits_length = i_num_block_per_row*i_num_block_per_col*i_num_colors;
    old_bass = new IntArr(0, i_blocks_bits_length);
    pv_tmp_color_bits = new IntVect;
    for (int c = 0; c < i_num_colors; c++) {
      pv_tmp_color_bits->push_back(0);
    }
    b_do_subtract_bg = p_osystem->settings().getBool("do_subtract_background", true);
    if (b_do_subtract_bg) {
      // Load the background matrix
      pm_background_matrix = new IntMatrix;
      import_matrix(pm_background_matrix, "backgrounds/background_matrix.txt");
    } else {
      pm_background_matrix = NULL;
    }
  }
}
        
/* *********************************************************************
    Deconstructor
 ******************************************************************** */
FIFOController::~FIFOController() {
  if (pi_old_frame_buffer != NULL) 
    delete [] pi_old_frame_buffer;
  if (p_fout != NULL) 
    fclose(p_fout);
  if (p_fin != NULL) 
    fclose(p_fin);
  if (old_bass != NULL)
    delete old_bass;
  if (pv_tmp_color_bits != NULL)
    delete pv_tmp_color_bits;
}
        
/* *********************************************************************
    This is called on every iteration of the main loop. It is resposible 
    passing the framebuffer and the RAM content to whatever AI module we 
    are using, and applying the returned actions.
 * ****************************************************************** */
void FIFOController::update() {
  Action player_a_action, player_b_action;
  // 0- See if we are skipping this frame
  if (i_skip_frames_counter < i_skip_frames_num) {
    // skip this frame
    i_skip_frames_counter++;
    if (e_previous_a_action == RESET || e_previous_b_action == RESET ) {
      player_a_action = PLAYER_A_NOOP;
      player_b_action = PLAYER_B_NOOP;
      e_previous_a_action = PLAYER_A_NOOP;
      e_previous_b_action = PLAYER_B_NOOP;
    } else {
      player_a_action = e_previous_a_action;
      player_b_action = e_previous_b_action;
    }
  } else {
    // don't skip this frame
    i_frame_number++;
    if (b_display_screen) {
      copy_framebuffer();
      if (b_use_bass) {
        plot_extracted_grid();
        display_bass_screen();
      }
      export_screen(pm_screen_matrix);
      display_screen();
    }
    i_skip_frames_counter = 0;
    // 1- Send the updated pixels in the screen through the pipe
    string final_str = "";
    if (b_send_console_ram) {
      // 1.1 - Get the ram content (128 bytes)
      for(int i = 0; i< 128; i++) {
        char buffer[5];
        int ram_byte = read_ram(i);
        sprintf (buffer, "%03i", ram_byte);
        final_str += buffer;
      }
    }
    if (b_send_screen_matrix) {
      if (b_use_bass) {
        if (!b_display_screen) // Only copy framebuffer once
          copy_framebuffer();
        generateBassRep();
        // plot_extracted_grid();
        // Send the abstract bass representation

        // Send the full binary
        for(int i = 0; i < i_blocks_bits_length; i++) {
          if ((*old_bass)[i])
            final_str += '1';
          else
            final_str += '0';
        }
      } else {
        // The next section is taken from FrameBufferSoft
        for (int i = 0; i < i_screen_width * i_screen_height; i++) {
          uInt8 v = pi_curr_frame_buffer[i];
          if (v != pi_old_frame_buffer[i]) {
            char buffer[50];
            int ind_j = i / i_screen_width;
            int ind_i = i - (ind_j * i_screen_width);
            sprintf (buffer, "%03i%03i%03i", ind_i, ind_j, v);
            final_str += buffer;
            pi_old_frame_buffer[i] = v;
          }
        }
      }
    } else {
      final_str += "NADA";
    }
    final_str += "\n\0";
    fputs(final_str.c_str(), p_fout);
    fflush (p_fout);
		

    // 2- Read the new action from the pipe
    // the action is sent as player_a_action,player_b_action
    char in_buffer [50];
    fgets (in_buffer, 50, p_fin);
    char * token = strtok (in_buffer,",\n");
    player_a_action = (Action)atoi(token);
    token = strtok (NULL,",\n");
    player_b_action = (Action)atoi(token);
  }
  e_previous_a_action = player_a_action;
  e_previous_b_action = player_b_action;
  apply_action(p_global_event_obj, player_a_action, player_b_action);
}


/* *********************************************************************
    Copies the content of the framebufer to pm_screen_matrix
 * ****************************************************************** */
void FIFOController::copy_framebuffer(void) {
    // This code section is taken from FrameBufferSoft
    int ind_i, ind_j;
    for (int i = 0; i < i_screen_width * i_screen_height; i++) {
        uInt8 v = pi_curr_frame_buffer[i];
        ind_i = i / i_screen_width;
        ind_j = i - (ind_i * i_screen_width);
        (*pm_screen_matrix)[ind_i][ind_j] = v;
    }
}

void FIFOController::export_screen(const IntMatrix * screen_matrix) {
	// Export the current screen to a PNG file
	ostringstream filename;
	char buffer [50];
	sprintf (buffer, "%09lld", i_frame_number);
	filename << "external/ale_v0.1/exported_screens/frame_" << buffer << ".png";
	p_osystem->p_export_screen->save_png(screen_matrix, filename.str());
}

void FIFOController::display_screen() {
	ostringstream filename;
	char buffer [50];
	sprintf (buffer, "%09lld", i_frame_number);
        filename << "external/ale_v0.1/exported_screens/frame_" << buffer << ".png";
        p_osystem->p_display_screen->display_png(filename.str());
        remove(filename.str().c_str());
}

void FIFOController::display_bass_screen() {
	ostringstream filename;
	char buffer [50];
	sprintf (buffer, "%09lld", i_frame_number);
        filename << "external/ale_v0.1/exported_screens/bass_frame_" << buffer << ".png";
        p_osystem->p_display_screen->display_bass_png(filename.str());
        remove(filename.str().c_str());
}


void FIFOController::generateBassRep(void) {
  int full_vect_index = 0;
  int i, j, c, bit_val;
  // Get the color-bits for all blocks into our temp array,
  // Also, generate the first part of the feature-vector
  for (c = 0; c < i_num_colors; c++) {
    (*pv_tmp_color_bits)[c] = 0;
  }

  for (i = 0; i < i_num_block_per_row; i++) {
    for (j = 0; j < i_num_block_per_col; j++) {	
      get_color_ind_from_block(i, j, pv_tmp_color_bits);
      for (c = 0; c < i_num_colors; c++) {
        bit_val = (*pv_tmp_color_bits)[c];
        (*old_bass)[full_vect_index] = bit_val;
        full_vect_index++;
      }
    }
  }
  // Now we'll get the crossproduct of all the bits
  // for (int i = 0; i < i_blocks_bits_length; i++) {
  //   //      Possible major oprimization:
  //   //        if ((*pv_tmp_fv_first_part)[i]  == 0) {
  //   //            full_vect_index += (i_ram_bits_length - (i + 1));
  //   //            continue;
  //   //        }
  //   for (int j = i + 1; j < i_blocks_bits_length; j++) {
  //     if ((*pv_tmp_fv_first_part)[i]  == 1 &&
  //         (*pv_tmp_fv_first_part)[j] == 1) { 
  //       add_one_index_to_feature_map(full_vect_index);
  //     }
  //     full_vect_index++;
  //   }
  // }
  // assert(full_vect_index == i_base_length);
}

/* *********************************************************************
   COPIED from grid_screen_agent

	Given the [i,j] indecies of a block, generates a integer vector 
	color_inds, such that color_inds[c] = 1 if the color c exists in 
	the block.
******************************************************************** */
void FIFOController::get_color_ind_from_block(int i, int j, IntVect* color_bits) {
  // reset the given vector all to zero
  for (int c = 0; c < i_num_colors; c++) {
    (*color_bits)[c] = 0;
  }
  assert (i >= 0 && i < i_num_block_per_row);
  assert (j >= 0 && j < i_num_block_per_col);
  int img_i = i * i_block_height;
  int img_j = j * i_block_width;
  for (int block_row = 0; block_row < i_block_height; block_row++) {
    for (int block_col = 0; block_col < i_block_width; block_col++) {
      int y = img_i + block_row;
      int x = img_j + block_col;
      assert (y >= 0 && y < i_screen_height);
      assert (x >= 0 && x < i_screen_width);
      int color_ind = (*pm_screen_matrix)[y][x];
      if (b_do_subtract_bg && color_ind == (*pm_background_matrix)[y][x])
        continue;	// background pixel
      assert(color_ind >= 0 && color_ind < 256);
      assert(color_ind % 2 == 0);
      int color_bit = pi_eight_bit_pallete[color_ind];
      (*color_bits)[color_bit] = 1;
    }
  }
}

void FIFOController::plot_extracted_grid() {
  int box_len = 4;	// Eaxh block will be plotted in 5x5 pixels
  // Initialize the plot matrix
  IntMatrix* grid_plot = new IntMatrix; 
  int plot_rows = (i_num_block_per_row * box_len) + i_num_block_per_row + 1;
  int plot_cols = (i_num_block_per_col * box_len) + i_num_block_per_col + 1;
  for (int i = 0; i < plot_rows; i++) {
    IntVect row;
    for (int j = 0; j < plot_cols; j++) {
      row.push_back(256 + BLACK_COLOR_IND);
    }
    grid_plot->push_back(row);
  }
  // Draw the white borders
  int row_num = 0;
  for (int i = 0; i <= i_num_block_per_row; i++) {
    for (int col_num = 0; col_num < plot_cols; col_num++) {
      (*grid_plot)[row_num][col_num] = 256 + WHITE_COLOR_IND;
    }
    row_num += (box_len + 1);
  }
  int col_num = 0;
  for (int j = 0; j <= i_num_block_per_col; j++) {
    for (int row_num = 0; row_num < plot_rows; row_num++) {
      (*grid_plot)[row_num][col_num] = 256 + WHITE_COLOR_IND;
    }
    col_num += (box_len + 1);
  }
	
  // Fill-in the grids with color
  int curr_row = 1;
  IntVect block_colors;
  for (int i = 0; i < i_num_block_per_row; i++) {
    int curr_col = 1;
    for (int j = 0; j < i_num_block_per_col; j++) {	
      // 1- Get the colors in this block
      get_color_ind_from_block(i, j, pv_tmp_color_bits);
      block_colors.clear();
      for (int c = 0; c < i_num_colors; c++) {
        if ((*pv_tmp_color_bits)[c]) {
          block_colors.push_back(256 + SECAM_COLOR_IND + c);
        }
      }
      // 2- plot a 4x4 box containing all these colors, or leave it
      //	  black if there are no colors
      if (block_colors.size() > 0) {
        int curr_color_ind = 0;
        for (int box_i = 0; box_i < box_len; box_i++) {
          for (int box_j = 0; box_j < box_len; box_j++) {
            (*grid_plot)[curr_row+box_i][curr_col+box_j] =
              block_colors[curr_color_ind];
            curr_color_ind++;
            if (curr_color_ind == block_colors.size()) {
              curr_color_ind = 0;
            }
          }
        }

      } 
      curr_col += (box_len + 1);
    }
    curr_row += (box_len + 1);
  }
	
  // Plot the matrix
  ostringstream filename;
  char buffer [50];
  sprintf (buffer, "%09lld", i_frame_number);
  filename << "external/ale_v0.1/exported_screens/bass_frame_" << buffer << ".png";
  p_osystem->p_export_screen->export_any_matrix(grid_plot, filename.str());
  delete grid_plot;
}


// A map from color id's to a number between 0, 7
uInt32 FIFOController::pi_eight_bit_pallete [256] = {
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0, 
  0, 0, 1, 0, 2, 0, 3, 0, 
  4, 0, 5, 0, 6, 0, 7, 0
};
