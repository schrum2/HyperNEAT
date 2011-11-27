/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 * *****************************************************************************/

#ifndef SELF_DETECTION_AGENT_H
#define SELF_DETECTION_AGENT_H

#include <deque>
#include "common_constants.h"
#include <boost/unordered_set.hpp>
#include "player_agent.h"
#include <set>

// Search a map for a key and returns default value if not found
template <typename K, typename V>
V GetWithDef(const std::map <K,V> & m, const K & key, const V & defval ) {
   typename std::map<K,V>::const_iterator it = m.find( key );
   if ( it == m.end() ) {
      return defval;
   }
   else {
      return it->second;
   }
};

// Calculates the information entropy of a random variable whose values and
// frequencies are provided by the map m.
template <typename K>
float compute_entropy(const std::map <K,int> & m, int count_sum) {
  float velocity_entropy = 0;
  typename std::map<K,int>::const_iterator it;
  for (it=m.begin(); it!=m.end(); ++it) {
    int count = it->second;
    float p_x = count / (float) count_sum;
    velocity_entropy -= p_x * log(p_x);
  }
  return velocity_entropy;
};

struct point {
  int x, y;

  point(int _x, int _y) { x = _x; y = _y; };

  bool operator< (const point& other) const {
    return x < other.x || (x == other.x && y < other.y);
  };
};

/* The blob is a region of contiguous color found in the game screen. */
struct Blob {
  int color;            // Color of this blob
  set<point> mask;      // The pixels composing this blob
  set<long> neighbors;  // Neighboring blob ids
  int x_min, x_max, y_min, y_max; // Bounding box of blob region
  int x_velocity, y_velocity; // Velocity of the blob
  long parent_id; long child_id;  // Pointers to ourself in the last and next timestep
  long id;               // Used for the comparator function. Should be unique.

  Blob();
  Blob(int _color, long _id);

  void update_minmax(int x, int y);
  void add_pixel(int x, int y);
  void add_neighbor(long neighbor_id);

  // Capture all the points associated with another blob
  void consume(const Blob& other);

  // Computes our velocity relative to a given blob. Velocity is based on
  // centroids of both blobs.
  void compute_velocity(const Blob& other);

  // Computes the euclidean distance between the blobs centroids
  float get_centroid_dist(const Blob& other);

  // Computes the percentage of pixels that overlap between two blobs.
  // Note that color is not taken into consideration!
  float get_percentage_absolute_overlap(const Blob& other);

  // Normalizes the two blobs according to the x_min, y_min and computes
  // a pixel overlap. Does not consider the color of the blobs.
  float get_percentage_relative_overlap(const Blob& other);

  // Spits out an all things considered blob match. Takes into account:
  // 1. color 2. distance 3. overlap 4. area 5. density
  float get_aggregate_blob_match(const Blob& other);

  // Find the blob that most closely resembles this blob. Do not consider
  // any of the blobs in the excluded set.
  long find_matching_blob(map<long,Blob>& blobs, set<long>& excluded);

  void check_bounding_box();

  void to_string();

  bool operator< (const Blob& other) const {
    return id < other.id;
  };
};

/* A composite object is an object composed of blobs. */
struct CompositeObject {
  long id;
  set<long> blob_ids;
  int x_velocity, y_velocity;
  int x_min, x_max, y_min, y_max; // Bounding box

  CompositeObject();
  CompositeObject(int x_vel, int y_vel, long _id);
  ~CompositeObject();

  void update_minmax(const Blob& b);
  void add_blob(const Blob& b);

  // Computes the bounding box based on all the current objects.
  void compute_boundingbox(map<long,Blob>& blob_map);

  // Attempts to expand the composite object by looking for any blobs who are
  // connected and have the same velocity
  void expand(map<long,Blob>& blob_map);

  // Computes a pixel match between this object and another
  float get_pixel_match(const CompositeObject& other);

  void check_bounding_box(map<long,Blob>& blob_map);
};

/* A prototype represents a class of objects. */
struct Prototype {
  set<long> obj_ids; // List of ids of objects belonging to this class
  set<point> mask;
  int x_min, x_max, y_min, y_max; // Bounding box
  
  Prototype (CompositeObject& obj, map<long,Blob>& blob_map);

  float get_pixel_match(CompositeObject& other, map<long,Blob>& blob_map);
};


class SelfDetectionAgent : public PlayerAgent {
 public:
  SelfDetectionAgent(GameSettings* _game_settings, OSystem* _osystem);
        
  // Returns a random action from the set of possible actions
  virtual Action agent_step(const IntMatrix* screen_matrix, 
                            const IntVect* console_ram, 
                            int frame_number);

  // Given a screen it will do object detection on that screen and
  // return the results.
  void process_image(const IntMatrix* screen_matrix, Action a);

  virtual void display_screen(const IntMatrix& screen_matrix);

  void find_connected_components(const IntMatrix& screen_matrix, map<long,Blob>& blobs);

  // Matches blobs found in the current timestep with those from
  // the last time step
  void find_blob_matches(map<long,Blob>& blobs);

  // Merges blobs together into composite objects
  void merge_blobs(map<long,Blob>& blobs);

  // Updates objects when a new timestep arrives
  void update_existing_objs();

  // Merges objects together into classes of objects
  void merge_objects(float similarity_threshold);

  // Looks through objects attempting to find one that we are controlling
  long identify_self();

  int get_num_regions(IntMatrix& regions);
  virtual void plot_regions(IntMatrix& screen_matrix);

  int screen_width, screen_height;
  int max_history_len;
  deque<IntMatrix> screen_hist;
  deque<Action> action_hist;

  int curr_num_regions;
  int prev_num_regions;
  IntMatrix region_matrix;

  long blob_ids;
  long obj_ids;

  map<long,Blob>            curr_blobs;      // Map of blob ids to blobs for the current frame
  map<long,CompositeObject> composite_objs;  // Map of obj ids to objs for the current frame
  vector<Prototype>         obj_classes;     // Classes of objects

  deque<map<long,Blob> >  blob_hist;
  deque<RegionObjectList> raw_object_hist;
  deque<RegionObjectList> merged_object_hist;

  // Parameters used for shape tracking
  float f_max_perc_difference;
  int i_max_obj_velocity;
  float f_max_shape_area_dif;
};

#endif




