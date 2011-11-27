/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  blob_class.h
 *
 *  Implementation of the BlobClass class, which represents a class of  objects
 *	detected in game screens. This should not be confused with ClassShape,
 *  which represents a single shape belonging to a BlobClass
 **************************************************************************** */

#ifndef BLOB_CLASS_H
#define BLOB_CLASS_H

#include "common_constants.h"
#include "blob_object.h"
#include "region_manager.h"

class BlobClass {
    /* *************************************************************************
        Represents a class of  RegionObjects detected in game screens. 
        
        Member variables:
            - pv_reg_objects        A list of RegionObject's, each representing
                                    one possible shape that belongs to this
                                    class
            - i_num_frames_on_scr  How many frames we have seen any instance of
                                    this class, in our samples. We use this to
                                    weed out "noise" classes that only appear
                                    in a few instance
            - i_screen_pos_x_min    min_x, max_x, min_y, max_y
              i_screen_pos_x_max    values for  positions where
              i_screen_pos_y_min    we detected this class. This is used to weed
              i_screen_pos_y_max    out classes that do not really move onscreen
            - i_discovered_on_frame  The frame number on which the first shape 
                                    of this class was discovered
    ************************************************************************* */

    public:
        BlobClass();
		virtual ~BlobClass();

        /* *********************************************************************
		 * Adds (a copy) of the shape to the add it to the the representative 
		 * objects of this class
         ******************************************************************** */
		void add_object(RegionObject* reg_object, int frame_num, 
						string discovery_txt);
		 
        /* *********************************************************************
		 * Given a new object that belongs to this class, updates our on-screen
         * boundaries.
		 ******************************************************************** */
		void update_screen_boudnaries(const RegionObject* reg_object);
        
		/* *********************************************************************
		 * Returns true if the given object belongs to this class.
         * When check_flipped_horizentally is true, we check if the flipped 
         * version of the object belongs to this class. Same deal with
         *  check_flipped_vertically, and allow_different_size
		 ******************************************************************** */
		bool belongs(const RegionObject* reg_object, bool allow_different_size,
						bool check_flipped_horizentally, 
						bool check_flipped_vertically, 
						float max_shape_area_dif) const;

		/* *********************************************************************
		 * Adds one to the i_num_frames_on of the corresponding RegionObject 
		 *  in our pv_rep_objects 
		 ******************************************************************** */
		void increase_obj_onscreen_count(const RegionObject* reg_object);
		
		/* *********************************************************************
		 * Merges this class with the given class
		 ******************************************************************** */
		void merge_with(const BlobClass* other_class);
		
		/* *********************************************************************
		 * Goes through all the region-objects assigned to this class, and 
		 * updates their i_instance_of_class
		 ******************************************************************** */
		void update_class_number(int new_num);

		/* *********************************************************************
		 * Returns the minimum (percentage) distance between the objects of two
		 * classes
		 * Returns false if the two classes donot have a well-defined distance
		 ******************************************************************** */
		bool get_min_distance(	const BlobClass* other_class, 
								float max_shape_area_dif, 
								float& min_dist) const;
		
		/* *********************************************************************
		 * Accessor method
		 ******************************************************************** */
		 const RegionObjectList* get_reg_objects(void) const;

		/* *********************************************************************
		 * Returns a string containing general info about this class. 
		 * This is used to save the class info, when plotting the blob-classes
		 ******************************************************************** */
		string get_info_text(int num_frames) const;

		RegionObjectList* pv_reg_objects;
		int i_num_frames_on_scr;
		int i_screen_pos_x_min;
		int i_screen_pos_x_max;
		int i_screen_pos_y_min;
		int i_screen_pos_y_max;
		int i_discovered_on_frame;

};
		
		
		
		
#endif
