/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  internal_controller.h
 *
 *  The implementation of the InternalController class, which is a subclass of 
 * GameConroller, and is resposible for sending the Screens/RAM content to 
 * whatever player_agent we are using to control the game
 **************************************************************************** */

#ifndef INTERNAL_CONTROLLER_H
#define INTERNAL_CONTROLLER_H


#include "common_constants.h"
#include "game_controller.h"
#include "player_agent.h"
#include "game_settings.h"


class InternalController : public GameController {
    /* *************************************************************************
        This is a subclass of GameConroller, and is resposible for sending the 
        Screens/RAM content to whatever player_agent we are using to control 
       the game, and apply the actions that are sent back
.
        
        Instance Variables:
        - m_screen_matrix;		2D Matrix containing the screen pixel colors
        - v_ram_content			Content of the emulator RAM
        - p_player_agent_right  The PlayerAgent object resposible for 
        - p_player_agent_left   controlling the right/left player
        - p_game_settings       Pointer to the related GameSettings object 
		- i_frame_number		Counts the frames (ignores the skipped frames)

    ************************************************************************* */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        InternalController(OSystem* _osystem);
        
        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
        virtual ~InternalController();
        
        /* *********************************************************************
            This is called on every iteration of the main loop. It is resposible 
            passing the framebuffer and the RAM content to whatever AI module we 
            are using, and applying the returned actions.
         * ****************************************************************** */
        virtual void update();

        inline PlayerAgent* getPlayerAgentLeft(void) {return p_player_agent_left;};

    protected:
        /* *********************************************************************
            Copies the content of the framebufer to pm_screen_matrix
         * ****************************************************************** */
        void copy_framebuffer(void);
        
        /* *********************************************************************
            Copies the content of RAM to pv_ram_content
         * ****************************************************************** */
        void copy_ram_content(void);
        
        
        IntMatrix* pm_screen_matrix; // 2D Matrix containing screen pixel colors
        IntVect* pv_ram_content;			// Content of the emulator RAM
        PlayerAgent* p_player_agent_right;	// The PlayerAgent object resposible  
        PlayerAgent* p_player_agent_left;	// for controlling right/left player
        GameSettings* p_game_settings;		// Pointer to a GameSettings object
		int i_frame_number;					// Counts the frames 
											// (ignores the skipped frames)
};
#endif
