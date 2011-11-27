/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  fifo_controller.h
 *
 *  The implementation of the FIFOController class, which is a subclass of 
 * GameConroller, and is resposible for sending the Screens/RAM content to 
 * whatever external program we are using through FIFO pipes, and apply the 
 * actions that are sent back
 **************************************************************************** */

#ifndef FIFO_CONTROLLER_H
#define FIFO_CONTROLLER_H


#include "common_constants.h"
#include "game_controller.h"

class FIFOController : public GameController {
    /* *************************************************************************
        This is a subclass of GameConroller, and is resposible for sending the 
        Screens/RAM content to whatever external program we are using through 
        FIFO pipes, and apply the actions that are sent back
.
        
        Instance Variables:
        - pi_old_frame_buffer;  // Copy of frame buffer. Used to detect and
                                // only send the changed pixels
        - p_fout;               // Output Pipe
        - p_fin;                // Input Pipe


    ************************************************************************* */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        FIFOController(OSystem* _osystem);
        
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~FIFOController();
        
        /* *********************************************************************
            This is called on every iteration of the main loop. It is resposible 
            passing the framebuffer and the RAM content to whatever AI module we 
            are using, and applying the returned actions.
         * ****************************************************************** */
        virtual void update();

    protected:
        /* *********************************************************************
            Copies the content of the framebufer to pm_screen_matrix
         * ****************************************************************** */
        void copy_framebuffer(void);
        void export_screen(const IntMatrix * screen_matrix);
        void display_screen();
        void display_bass_screen();

        bool b_display_screen;      // Should we display the screen?
        bool b_use_bass;            // Use abstract bass representation
        IntMatrix* pm_screen_matrix; // 2D Matrix containing screen pixel colors
        int* pi_old_frame_buffer;   // Copy of frame buffer. Used to detect and
                                    // only send the changed pixels
        FILE* p_fout;               // Output Pipe
        FILE* p_fin;                // Input Pipe
        long long i_frame_number;   // Count of frames

        /* *********************************************************************
           SECAM/BASS stuff
         * ****************************************************************** */
        // Converts our normal pixel representation into a SECAM/BASS one
        static const int i_num_colors = 8;
        void generateBassRep(void);
        void get_color_ind_from_block(int i, int j, IntVect* color_bits);
        void plot_extracted_grid();

        // Resolution of our representation
        int i_num_block_per_row;
        int i_num_block_per_col;
        int i_block_height;
        int i_block_width;
        int i_blocks_bits_length;
        bool b_do_subtract_bg;
        IntMatrix* pm_background_matrix;
        IntVect* pv_tmp_color_bits;        
        static uInt32 pi_eight_bit_pallete[256];
        IntArr* old_bass;
};
#endif
