/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 * *****************************************************************************/

#ifndef VISUAL_PROCESSOR_H
#define VISUAL_PROCESSOR_H

#include <deque>
#include "../player_agents/common_constants.h"
#include <boost/unordered_set.hpp>
#include <set>
#include <map>
#include "display_screen.h"
#include "../player_agents/game_settings.h"
#include "../emucore/OSystem.hxx"

// Operations for working on pixel masks
static void add_pixel(int width, int height, int relx, int rely, vector<char>& mask) {
    int block = (width * rely + relx) / 8;
    int indx_in_block = (width * rely + relx) % 8;
    mask[block] = mask[block] | (1 << (7-indx_in_block));
};

static bool get_pixel(int width, int height, int relx, int rely, vector<char>& mask) {
    int block = (width * rely + relx) / 8;
    int indx_in_block = (width * rely + relx) % 8;
    return mask[block] & (1 << (7-indx_in_block));
};
// End pixel mask operations

// Search a map for a key and returns default value if not found
template <typename K, typename V>
    V GetWithDef(const std::map <K,V> & m, const K & key, const V & defval ) {
    typename std::map<K,V>::const_iterator it = m.find( key );
    if (it == m.end()) {
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

// Counts the number of 1 bits in a char. Taken from stack overflow
const int oneBits[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
static int count_ones(unsigned char x) {
    int results;
    results = oneBits[x&0x0f];
    results += oneBits[x>>4];
    return results;
};

struct point {
    int x, y;

    point(int _x, int _y) { x = _x; y = _y; };

    bool operator< (const point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    };
};

/* A swath of color. Used as an intermediate data struct in blob detection. */
struct swath {
    int color;

    // Bounding box of swath
    int x_min, x_max, y_min, y_max; 

    // x,y locations for pixels it contains
    vector<int> x;
    vector<int> y;

    swath* parent;

    swath(int _color, int _x, int _y);

    void update_bounding_box(int x, int y);

    void update_bounding_box(swath& other);
};

/* The blob is a region of contiguous color found in the game screen. */
struct Blob {
    int color;            // Color of this blob
    set<long> neighbors;  // Neighboring blob ids
    vector<char> mask;    // Pixel mask -- we may want an array here
    int size;             // Number of pixels
    int x_min, x_max, y_min, y_max; // Bounding box of blob region
    int height, width;    // Width and height of the blob
    int x_velocity, y_velocity; // Velocity of the blob
    long parent_id; long child_id;  // Pointers to ourself in the last and next timestep
    long id;              // Used for the comparator function. Should be unique.

    Blob();
    Blob(int _color, long _id, int _x_min, int _x_max, int _y_min, int _y_max);
    ~Blob();

    void update_minmax(int x, int y);
    /* void add_pixel_abs(int absx, int absy); */
    /* void add_pixel_rel(int relx, int rely); */
    /* bool get_pixel_abs(int absx, int absy); */
    /* bool get_pixel_rel(int relx, int rely); */
    void add_neighbor(long neighbor_id);
    point get_centroid();

    // Computes our velocity relative to a given blob. Velocity is based on
    // centroids of both blobs.
    void compute_velocity(const Blob& other);

    // Computes the euclidean distance between the blobs centroids
    float get_centroid_dist(const Blob& other);

    // Spits out an all things considered blob match. Takes into account:
    // 1. color 2. distance 3. overlap 4. area 5. density
    float get_aggregate_blob_match(const Blob& other);

    // Find the blob that most closely resembles this blob. Do not consider
    // any of the blobs in the excluded set.
    long find_matching_blob(map<long,Blob>& blobs);

    void to_string(bool verbose=false, deque<map<long,Blob> >* blob_hist=NULL);

    bool operator< (const Blob& other) const {
        return id < other.id;
    };
};

/* A composite object is an object composed of blobs. */
struct CompositeObject {
    long id;
    set<long> blob_ids;
    vector<char> mask;
    int x_velocity, y_velocity;
    int x_min, x_max, y_min, y_max; // Bounding box
    int width, height;
    int size;
    int frames_since_last_movement;
    int age;  // # of timesteps since this object was discovered

    CompositeObject();
    CompositeObject(int x_vel, int y_vel, long _id);
    ~CompositeObject();

    void clean();
    // Updates the bounding box from a blob
    void update_bounding_box(const Blob& b);
    /* void add_pixel_rel(int relx, int rely); */
    void add_blob(const Blob& b);
    point get_centroid() { return point((x_max+x_min)/2,(y_max+y_min)/2); };

    // Attempts to expand the composite object by looking for any blobs who are
    // connected and have the same velocity
    void expand(map<long,Blob>& blob_map);

    // Builds the mask from the current set of blobs
    void computeMask(map<long,Blob>& blob_map);

    // Compares to see if this object and another objects masks are the same
    bool maskEquals(const CompositeObject& other);
    
    void to_string(bool verbose=false);
};

/* A prototype represents a class of objects. */
struct Prototype {
    int id;
    set<long> obj_ids; // List of ids of objects belonging to this class
    vector<char> mask;
    int width, height;
    int size;

    long seen_count;
    int frames_since_last_seen;
    int times_seen_this_frame;
    bool is_valid;
    float self_likelihood, alpha; // How likely is the prototype to be part of the "self"?
  
    Prototype(CompositeObject& obj, map<long,Blob>& blob_map);

    // How closely does an object resemble this prototype?
    float get_pixel_match(CompositeObject& obj);

    void to_string(bool verbose=false);
};


class VisualProcessor : public SDLEventHandler {
 public:
    VisualProcessor(OSystem* _osystem, GameSettings* _game_settings);
        
    // Given a screen it will do object detection on that screen and
    // return the results.
    void process_image(const IntMatrix* screen_matrix, Action a);

    virtual void display_screen(IntMatrix& screen_matrix);

    void plot_blobs(IntMatrix& screen_matrix);   // Plots the blobs on screen
    void plot_objects(IntMatrix& screen_matrix); // Plots the objects on screen
    void plot_prototypes(IntMatrix& screen_matrix); // Plots the prototypes on screen
    void plot_self(IntMatrix& screen_matrix);    // Plots the self blob
    void box_object(CompositeObject& obj, IntMatrix& screen_matrix, int color); // Draws a box around an object
    void box_blob(Blob& b, IntMatrix& screen_matrix, int color); // Draws a box around a blob  

    void find_connected_components(const IntMatrix& screen_matrix, map<long,Blob>& blobs);

    // Matches blobs found in the current timestep with those from
    // the last time step
    void find_blob_matches(map<long,Blob>& blobs);

    // Merges blobs together into composite objects
    void merge_blobs(map<long,Blob>& blobs);

    // Updates objects when a new timestep arrives
    void update_existing_objs();

    // Sanitize objects based on size and velocity
    void sanitize_objects();

    // Merges objects together into classes of objects
    void merge_objects(float similarity_threshold);

    // Looks through objects attempting to find one that we are controlling
    void identify_self();

    // Gives a point corresponding to the location of the self on the screen.
    // Assumes identify self has already been called.
    point get_self_centroid();

    // Returns true if a self object has been located.
    bool found_self();

    void handleSDLEvent(const SDL_Event& event);

    void printVelHistory(CompositeObject& obj);

    // Saves an image of the currently selected object -- this object should be the self
    void tagSelfObject();
    void loadSelfObject();
    void exportMask(int width, int height, vector<char>& mask, const string& filename);
    void importMask(int& width, int& height, vector<char>& mask, const string& filename);

    OSystem* p_osystem;
    GameSettings* game_settings;
    int screen_width, screen_height;
    int max_history_len;
    deque<IntMatrix> screen_hist;
    deque<Action> action_hist;
    deque<map<long,Blob> >  blob_hist;

    long blob_ids;
    long obj_ids;

    map<long,Blob>            curr_blobs;      // Map of blob ids to blobs for the current frame
    map<long,CompositeObject> composite_objs;  // Map of obj ids to objs for the current frame
    vector<Prototype>         obj_classes;     // Classes of objects

    long self_id; // ID of the object which corresponds to the "self"

    // Self objects which are manually identified. These are loaded up from saved files of the game
    vector<CompositeObject> self_objects; 

    /** Graphical display stuff **/
    long focused_entity_id; // The focused object is selected by a click
    int focus_level;     // Are we focusing on a blob/object/prototype?
    int display_mode;    // Which graphical representation should we display?
    bool display_self;   // Should the results of self detection be displayed?

    // Parameters used for shape tracking
    /* float f_max_perc_difference; */
    /* int i_max_obj_velocity; */
    /* float f_max_shape_area_dif; */

    //(piyushk)
    int prototype_ids;
    float prototype_value;
    set<int> free_colors;
};

#endif

