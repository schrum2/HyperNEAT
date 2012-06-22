/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  blob_object.h
 *
 *  Represents an object detected in a game screen.
 **************************************************************************** */

#ifndef BLOB_OBJECT_H
#define BLOB_OBJECT_H

#include "common_constants.h"
#include "../common/export_screen.h"

class BlobObject {
  /* *************************************************************************
     Represents an object detected in a game screen.
        
     Member variables:
     - i_center_x, i_center_y    coordinate of the *center* of the object 
     - i_velocity_x, i_velocity_y (dx, dy) velocity of a moving object
     - i_instance_of_class       The class number that this object is an
     instance of. -1 = undefined
     ************************************************************************* */

 public:
  BlobObject( int _center_x, int _center_y, 
              int _velocity_x = 0, int _velocity_y = 0, 
              int _instance_of_class = 0);

  /* *********************************************************************
     Returns the  distance between this object and the given object
     Returns the max(abs(distance_x), abs(distance_y))
     ******************************************************************** */
  int calc_distance_max_xy( BlobObject* other_obj);
        
  int i_center_x, i_center_y;
  int i_velocity_x, i_velocity_y;
  int i_instance_of_class;
        
};

class RegionObject : public BlobObject {
  /* *************************************************************************
     Represents an object detected in the region matrix.
     This is used in Class-Discovery 
        
     Member variables:
     - pm_shape_matrix		A binary 2D matrix containing the shape of 
     object
     - i_region_number		The region number that this object is
     associated with
     - i_width, i_height		Shape of the object
     - p_previous_object		The blob-object that corresponds to this 
     object in the previous frame
     - b_is_valid			True when this is a valid object. False if 
     region number assigned to this object has no
     actual pixles in the region_matrix
     - i_disc_frame_num		Number of frame in which this object
     was first discovered in
     - i_num_frames_on		Number of frames that we detected this 
     object on the screen.
     - str_disc_text         A text string, containing the reason for
     assignment of this object to the class
     it was assigned to. 
     ************************************************************************* */

 public:
  /* *********************************************************************
     Constructor
     This will generate an in_valid object. User should call
     extract_from_region_matrix to properly initialize this method
     ******************************************************************** */
  RegionObject ();
  RegionObject(const RegionObject& copyObj); // copy constructor
  RegionObject& operator=(const RegionObject& that); //assignment

  virtual ~RegionObject();

		
  /* *********************************************************************
     Given a region matrix (generated in region_manager),
     and a region number, extracts the object from the matrix
     Returns true if the extraction is sucessful
     ******************************************************************** */
  bool extract_from_region_matrix(const IntMatrix* region_matrix, 
                                  int region_num);
		
  /* *********************************************************************
     Calculates the pixel and percentage distance between the two 
     objects. Returns true if the distance is computable (i.e. the 
     objects have compatible shapes).
     ******************************************************************** */
  bool calc_distance(	const RegionObject* other_object, 
                        bool allow_different_size, float max_shape_area_dif,
                        int& pixel_distance, float& perc_distance);
							
  /* *********************************************************************
     Returns the Region-Object closest to this object in the given list of 
     objects.
     We do this by collecting a set of 'similar' objects that apear 
     'close' to the given object in the previous frame, and then return
     the one that is most similar.
     ******************************************************************** */
  RegionObject* get_closest_obj_in_list(
                                        const RegionObjectList* pv_obj_list,
                                        int list_length,
                                        float max_shape_area_dif,
                                        float max_perc_difference,
                                        int max_obj_velocity);
		
  /* *********************************************************************
     Save the shape_matrix as a PNG file
     ******************************************************************** */
  void plot(string& filename, const ExportScreen* p_export_screen) const;

  /* *********************************************************************
     Returns the string version of this object, so it can be imported 
     later. The string format is as follows:
     class_num,width,height|
     M[0,0], M[0,1], ..., M[0,n]|
     M[1,0], M[1,1], ..., M[1,n]|
     ...
     M[m,0], M[m,1], ..., M[m,n]|
     ******************************************************************** */
  string export_object_as_txt (void) const;
		
  /* *********************************************************************
   * Returns a string containing general info about this object. 
   * This is used to save object info, after class-discovery
   ******************************************************************** */
  string get_info_text(void) const;
		 		
  IntMatrix* pm_shape_matrix;
  int i_region_number;
  RegionObject* p_previous_object;
  bool b_is_valid;
  int i_disc_frame_num;
  int i_num_frames_on;
  string str_disc_text;
  int i_width, i_height;
		
}; 

/* *************************************************************************
   Compares two objects based on their area size. Returns returns true if 
   obj1 > obj2, false otherwise
   ************************************************************************* */
bool compare_objects_by_shape_size(	const RegionObject* obj1, 
                                        const RegionObject* obj2);

#endif 
