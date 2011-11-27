/* *****************************************************************************
 *  export_screen.h
 *
 *  The implementation of the ExportScreen class, which is responsible for 
 *  saving the screen matrix to an image file. 
 * 
 *  Note: Most of the code here is taken from Stella's Snapshot.hxx/cxx
 **************************************************************************** */

#ifndef EXPORT_SCREEN_H
#define EXPORT_SCREEN_H

#include <vector>
#include "Props.hxx"
#include "common_constants.h"

class OSystem;

class ExportScreen {
    /* *************************************************************************
        This class is responsible for saving the screen matrix to an image file. 

        
        Instance Variables:
            - pi_palette        An array containing the pallete
            - p_props           Pointer to a Properties object
            - p_osystem         pointer to the Osystem object
            - i_screen_width    Width of the screen
            - i_screen_height   Height of the screen
            - v_custom_pallete  Holds the rgb values for custom colors used
                                for drawing external info on the screen
    ************************************************************************* */
    public:
        /* *********************************************************************
            Constructor
         ******************************************************************** */
        ExportScreen(OSystem* osystem);

        /* *********************************************************************
            Deconstructor
         ******************************************************************** */
         virtual ~ExportScreen() {}
        
        /* *********************************************************************
            Sets the default pallete. This needs to be called before any
            export methods can be called.
         ******************************************************************** */
        virtual void set_palette(const uInt32* palette) {
            pi_palette = palette;
        }

        /* *********************************************************************
            Saves the given screen matrix as a PNG file
         ******************************************************************** */        
        void save_png(const IntMatrix* screen_matrix, const string& filename);

	/* *********************************************************************
		Saves the any matrix (not just the scree nmatrix)  as a PNG file
	 ******************************************************************** */        
	void export_any_matrix (	const IntMatrix* screen_matrix, 
							const string& filename) const;
	/* *********************************************************************
		Saves a float matrix as a heat plot png file
	 ******************************************************************** */        
	void save_heat_plot(FloatMatrix* pm_matrix, 
										 const string& filename);
										 
    protected:
        /* *********************************************************************
            Gets the RGB values for a given screen value from the current pallete
         ******************************************************************** */    
        void get_rgb_from_pallete(int val, int& r, int& g, int& b) const;

        /* *********************************************************************
            Initilizes the custom pallete
         ******************************************************************** */    
        void init_custom_pallete(void);
        void writePNGChunk(ofstream& out, char* type, uInt8* data, int size) const;
        void writePNGText(ofstream& out, const string& key, 
						 const string& text) const;

        const uInt32* pi_palette;
        const Properties* p_props;
        OSystem* p_osystem;
        int i_screen_width;      // Width of the screen
        int i_screen_height;     // Height of the screen
        vector< vector <int> > v_custom_pallete;
};

#endif
