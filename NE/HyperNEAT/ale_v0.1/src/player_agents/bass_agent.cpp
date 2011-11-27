#include "export_tools.h"
#include "game_controller.h"
#include "bass_agent.h"
#include "random_tools.h"
#include <sstream>

BassAgent::BassAgent(GameSettings* _game_settings, OSystem* _osystem):
  PlayerAgent(_game_settings, _osystem) {
  // Get the height and width of the screen
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  i_screen_width  = mediasrc.width();
  i_screen_height = mediasrc.height();
  i_num_colors = 8;
  b_do_subtract_bg = p_osystem->settings().getBool("do_subtract_background",true);
  i_num_block_per_row = p_osystem->settings().getInt("num_block_per_row", true);
  i_num_block_per_col = p_osystem->settings().getInt("num_block_per_col", true);
  if ( (i_screen_width % i_num_block_per_row != 0) ||
       (i_screen_height % i_num_block_per_col != 0) ) {
    cerr << "WARNING: Invalid i_num_block_per_row or i_num_block_per_col value."
         << "Screen-Width should be divisible by i_num_block_per_row. "
         << "Screen-Height should be divisible by i_num_block_per_col." 
         << endl;
    exit(-1);
  }
  // Height and width of each block in pixels
  i_block_height = i_screen_height / i_num_block_per_col;
  i_block_width = i_screen_width / i_num_block_per_row;
  i_blocks_bits_length = i_num_block_per_row*i_num_block_per_col*i_num_colors;

  pv_feature_vec = new IntArr(0, i_blocks_bits_length);
  pv_tmp_color_bits = new IntVect;
  for (int c = 0; c < i_num_colors; c++)
    pv_tmp_color_bits->push_back(0);
  if (b_do_subtract_bg) {
    // Load the background matrix
    pm_background_matrix = new IntMatrix;
    string romfile = p_osystem->settings().getString("rom_file",true);
    string bgfile("backgrounds/");
    bgfile += romfile.substr(romfile.find_last_of("/\\")+1);
    import_matrix(pm_background_matrix, bgfile);
  } else {
    pm_background_matrix = NULL;
  }
}

BassAgent::~BassAgent() {
  delete pv_tmp_color_bits;
  delete pv_feature_vec;
  if (pm_background_matrix) {
    delete pm_background_matrix;
  }
}

Action BassAgent::agent_step(const IntMatrix* screen_matrix,
                             const IntVect* console_ram,
                             int frame_number) {
  
  Action special_action = PlayerAgent::agent_step(screen_matrix, console_ram, frame_number);

  generate_feature_vec();

  displayScreen();

  if (special_action != UNDEFINED) {
    return special_action;  // We are resetting or in some sort of delay 
  }

  return agent_step(pv_feature_vec);
}

void BassAgent::displayScreen() {
  if (b_display_screen) {
    plot_extracted_grid(pv_feature_vec);
    display_bass_screen();
  }
}

Action BassAgent::agent_step(IntArr* pv_feature_vec) {
  return choice <Action> (p_game_settings->pv_possible_actions);
}

void BassAgent::on_end_of_game(void) {
  PlayerAgent::on_end_of_game();
  cout << "V(end) = " << f_curr_reward << endl;  
}

void BassAgent::generate_feature_vec(void) {
  int full_vect_index = 0;
  int x, y, c, bit_val;
  for (c = 0; c < i_num_colors; c++)
    (*pv_tmp_color_bits)[c] = 0;
  for (y = 0; y < i_num_block_per_col; y++) {	
    for (x = 0; x < i_num_block_per_row; x++) {
      get_color_ind_from_block(x, y, pv_tmp_color_bits);
      for (c = 0; c < i_num_colors; c++) {
        bit_val = (*pv_tmp_color_bits)[c];
        (*pv_feature_vec)[full_vect_index] = bit_val;
        full_vect_index++;
      }
    }
  }
}

void BassAgent::get_color_ind_from_block(int x, int y, IntVect* color_bits) {
  // reset the given vector all to zero
  for (int c = 0; c < i_num_colors; c++) {
    (*color_bits)[c] = 0;
  }

  assert (x >= 0 && x < i_num_block_per_row);
  assert (y >= 0 && y < i_num_block_per_col);
  int img_y = y * i_block_height;
  int img_x = x * i_block_width;
  for (int block_row = 0; block_row < i_block_height; block_row++) {
    for (int block_col = 0; block_col < i_block_width; block_col++) {
      int y = img_y + block_row;
      int x = img_x + block_col;
      assert (y >= 0 && y < i_screen_height);
      assert (x >= 0 && x < i_screen_width);
      int color_ind = (*pm_curr_screen_matrix)[y][x];
      if (b_do_subtract_bg && color_ind == (*pm_background_matrix)[y][x])
        continue;	// background pixel
      assert(color_ind >= 0 && color_ind < 256);
      assert(color_ind % 2 == 0);
      int color_bit = pi_eight_bit_pallete[color_ind];
      (*color_bits)[color_bit] = 1;
    }
  }
}


// A map from color id's to a number between 0, 7
uInt32 BassAgent::pi_eight_bit_pallete [256] = {
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


void BassAgent::display_bass_screen() {
  ostringstream filename;
  char buffer [50];
  sprintf (buffer, "%09lld", i_frame_counter);
  filename << "exported_screens/bass_frame_" << buffer << ".png";
  p_osystem->p_display_screen->display_bass_png(filename.str());
  remove(filename.str().c_str());
}

void BassAgent::plot_extracted_grid(IntArr* pv_feature_vec) {
  int box_len = 4;	// Eaxh block will be plotted in 5x5 pixels
  // Initialize the plot matrix
  IntMatrix grid_plot; 
  int plot_rows = (i_num_block_per_col * box_len) + i_num_block_per_col + 1;
  int plot_cols = (i_num_block_per_row * box_len) + i_num_block_per_row + 1;
  for (int y = 0; y < plot_rows; y++) {
    IntVect row;
    for (int x = 0; x < plot_cols; x++) {
      row.push_back(256 + BLACK_COLOR_IND);
    }
    grid_plot.push_back(row);
  }
  // Draw the white borders
  int row_num = 0;
  for (int i = 0; i <= i_num_block_per_col; i++) {
    for (int col_num = 0; col_num < plot_cols; col_num++) {
      grid_plot[row_num][col_num] = 256 + WHITE_COLOR_IND;
    }
    row_num += (box_len + 1);
  }
  int col_num = 0;
  for (int j = 0; j <= i_num_block_per_row; j++) {
    for (int row_num = 0; row_num < plot_rows; row_num++) {
      grid_plot[row_num][col_num] = 256 + WHITE_COLOR_IND;
    }
    col_num += (box_len + 1);
  }
	
  // Fill-in the grids with color
  int curr_row = 1;
  IntVect block_colors;
  for (int y = 0; y < i_num_block_per_col; y++) {
    int curr_col = 1;
    for (int x = 0; x < i_num_block_per_row; x++) {	
      // 1- Get the colors in this block
      //get_color_ind_from_block(x, y, pv_tmp_color_bits);
      block_colors.clear();
      for (int c = 0; c < i_num_colors; c++) {
        if ((*pv_feature_vec)[i_num_block_per_row * i_num_colors * y + i_num_colors * x + c]) {
          //if ((*pv_tmp_color_bits)[c]) {
          block_colors.push_back(256 + SECAM_COLOR_IND + c);
        }
      }
      // 2- plot a 4x4 box containing all these colors, or leave it
      //	  black if there are no colors
      if (block_colors.size() > 0) {
        int curr_color_ind = 0;
        for (int box_y = 0; box_y < box_len; box_y++) {
          for (int box_x = 0; box_x < box_len; box_x++) {
            grid_plot[curr_row+box_y][curr_col+box_x] =
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
  sprintf (buffer, "%09lld", i_frame_counter);
  filename << "exported_screens/bass_frame_" << buffer << ".png";
  p_osystem->p_export_screen->export_any_matrix(&grid_plot, filename.str());
}
