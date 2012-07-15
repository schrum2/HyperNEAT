/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 **************************************************************************** */

#include "visual_processor.h"
#include "random_tools.h"
#include "../emucore/m6502/src/System.hxx"
#include <limits>
#include <sstream>
#include <omp.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#ifdef MUNKRES
    #include "munkres.h" // Implementation of the Kuhn-Munkres algorithm
#endif

#define IMAGE_FILENAME "images"
#define SELF_IMAGE_PREFIX "selfimage-"
#define SELF_IMAGE_SUFFIX ".bin"
#define CLASS_IMAGE_PREFIX "classimage-"
#define CLASS_IMAGE_SUFFIX ".bin"

swath::swath(int _color, int _x, int _y) {
    color = _color;
    x_min = numeric_limits<int>::max();
    x_max = numeric_limits<int>::min();
    y_min = numeric_limits<int>::max();
    y_max = numeric_limits<int>::min();
    x.push_back(_x);
    y.push_back(_y);
    parent = NULL;
};

void swath::update_bounding_box(int x, int y) {
    x_min = min(x,x_min);
    y_min = min(y,y_min);
    x_max = max(x,x_max);
    y_max = max(y,y_max);
};

void swath::update_bounding_box(swath& other) {
    update_bounding_box(other.x_min, other.y_min);
    update_bounding_box(other.x_max, other.y_max);
};

PixelMask::PixelMask() : width(0), height(0), size(0)
{};

PixelMask::PixelMask(int width, int height) :
    width(width), height(height), size(0)
{
    // Do not need extra byte if modulo 8 is zero
    // TODO: Do we need the +1?
    for (int i=0; i<width*height/8 + 1; ++i)
        pixel_mask.push_back(0x0);
};

void PixelMask::set_pixel(int x, int y, bool val) {
    // Does the requested pixel already have the requested value?
    if (get_pixel(x, y) == val)
        return;
    
    int block = (width * y + x) / 8;
    int indx_in_block = (width * y + x) % 8;
    assert(block < pixel_mask.size());

    if (val) {
        pixel_mask[block] = pixel_mask[block] | (1 << (7-indx_in_block));
        size++;
    } else {
        pixel_mask[block] = pixel_mask[block] & (0 << (7-indx_in_block));
        size--;
    }
};

bool PixelMask::get_pixel(int x, int y) {
    int block = (width * y + x) / 8;
    int indx_in_block = (width * y + x) % 8;
    assert(block < pixel_mask.size());
    return pixel_mask[block] & (1 << (7-indx_in_block));
};

bool PixelMask::equals(const PixelMask& other) {
    if (width != other.width || height != other.height || pixel_mask.size() != other.pixel_mask.size())
        return false;
    for (int i=0; i<pixel_mask.size(); ++i)
        if (pixel_mask[i] != other.pixel_mask[i])
            return false;
    return true;
};

int PixelMask::get_pixel_overlap(const PixelMask& other) {
    if (width != other.width || height != other.height ||
        pixel_mask.size() != other.pixel_mask.size()) {
        return 0;
    }

    int overlap = 0;
    for (int i=0; i<pixel_mask.size(); ++i) {
        overlap += count_ones(pixel_mask[i] & other.pixel_mask[i]);
    }
    return overlap;
};

float PixelMask::get_pixel_overlap_percent(const PixelMask& other) {
    return get_pixel_overlap(other) / (float) size;
};

void PixelMask::clear() {
    pixel_mask.clear();
    width = height = size = 0;
};

void PixelMask::to_string() {
    printf("Pixel mask: Width %d  Height %d Size %d", width, height, size);
    for (int y=0; y<height; ++y) {
        printf("\n");
        for (int x=0; x<width; ++x) {
            printf("%d",get_pixel(width, height));
        }
    }
    printf("\n");
};

Blob::Blob () {
    id = -1;
};

Blob::Blob (int color, long id, int x_min, int x_max, int y_min, int y_max) :
    id(id), color(color), x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max),
    mask(x_max - x_min + 1, y_max - y_min + 1),
    x_velocity(0), y_velocity(0), parent_id(-1), child_id(-1)
{
};

void Blob::add_neighbor(long neighbor_id) {
    neighbors.insert(neighbor_id);
};

point Blob::get_centroid() {
    return point((x_min+x_max)/2,(y_min+y_max)/2);
};

void Blob::compute_velocity(const Blob& other) {
    y_velocity = (y_max + y_min) / 2.0 - (other.y_max + other.y_min) / 2.0;
    x_velocity = (x_max + x_min) / 2.0 - (other.x_max + other.x_min) / 2.0;
};

float Blob::get_centroid_dist(const Blob& other) {
    float y_diff = (y_max + y_min) / 2.0 - (other.y_max + other.y_min) / 2.0;
    float x_diff = (x_max + x_min) / 2.0 - (other.x_max + other.x_min) / 2.0;
    float euclid_dist = pow(y_diff*y_diff + x_diff*x_diff,.5);
    return euclid_dist;
};

float Blob::get_aggregate_blob_match(const Blob& other) {
    int color_diff  = color == other.color;
    float dist_diff = get_centroid_dist(other);
    float size_ratio = min(mask.size, other.mask.size) / (float) max(mask.size, other.mask.size);

    float normalized_dist_diff = max(0.0f, 1 - (dist_diff * dist_diff / 625.0f));
    float normalized_size_diff = size_ratio;
    float match = (color_diff + normalized_size_diff + normalized_dist_diff) / 3.0f;
    return match;
};

long Blob::find_matching_blob(map<long,Blob>& blobs) {
    long best_match_id = -1;
    float best_match_score = 0;
    for (map<long,Blob>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
        Blob& b = it->second;

        float match = get_aggregate_blob_match(b);
        if (match > best_match_score || best_match_id < 0) {
            best_match_id = b.id;
            best_match_score = match;
        }
    }
    if (best_match_score < .667) {
        return -1;
    }

    return best_match_id;
};

void Blob::to_string(bool verbose, deque<map<long,Blob> >* blob_hist) {
    printf("Blob: %p BB: (%d,%d)->(%d,%d) Size: %d Col: %d\n",this,x_min,y_min,x_max,y_max,
           mask.size,color);

    if (verbose) {
        mask.to_string();
        
        if (blob_hist != NULL) {
            printf("Velocity History: \n");
            Blob* b = this;
            // Get the velocity history of this blob
            int blob_history_len = 1;
            while (b->parent_id >= 0 && blob_history_len < blob_hist->size()) {
                // Push back the velocity
                pair<int,int> vel(b->x_velocity, b->y_velocity);
                printf("Age %d Blob %ld Vel (%d,%d)\n", blob_history_len-1, b->id, b->x_velocity, b->y_velocity);
                blob_history_len++;

                // Get the parent
                map<long,Blob>& old_blobs = (*blob_hist)[blob_hist->size() - blob_history_len];
                long parent_id = b->parent_id;
                assert(old_blobs.find(parent_id) != old_blobs.end());
                b = &old_blobs[parent_id];
            }
        }
    }
};

CompositeObject::CompositeObject() {
    id = -1;
    frames_since_last_movement = 0;
};

CompositeObject::CompositeObject(int x_vel, int y_vel, long id) :
    id(id), x_velocity(x_vel), y_velocity(y_vel), frames_since_last_movement(0), age(0)
{
    x_min = numeric_limits<int>::max();
    x_max = numeric_limits<int>::min();
    y_min = numeric_limits<int>::max();
    y_max = numeric_limits<int>::min();
};

void CompositeObject::clear() {
    x_min = numeric_limits<int>::max();
    x_max = numeric_limits<int>::min();
    y_min = numeric_limits<int>::max();
    y_max = numeric_limits<int>::min();
  
    blob_ids.clear();
    mask.clear();
};

void CompositeObject::update_bounding_box(const Blob& b) {
    x_min = min(b.x_min, x_min);
    x_max = max(b.x_max, x_max);
    y_min = min(b.y_min, y_min);
    y_max = max(b.y_max, y_max);
};

void CompositeObject::add_blob(const Blob& b) {
    assert(b.x_velocity == x_velocity);
    assert(b.y_velocity == y_velocity);

    blob_ids.insert(b.id);
    update_bounding_box(b);
};

void CompositeObject::expand(map<long,Blob>& blob_map) {
    set<long> checked, remaining;
    set<long>::iterator it;

    // Insert all current blob ids
    for (it=blob_ids.begin(); it!=blob_ids.end(); ++it)
        remaining.insert(*it);

    while (!remaining.empty()) {
        it = remaining.begin();
        long b_id = *it;
        remaining.erase(it);

        // Skip this if we've already checked it
        if (checked.find(b_id) != checked.end())
            continue;

        assert(blob_map.find(b_id) != blob_map.end());
        Blob& b = blob_map[b_id];

        // Add this to our blob set if it has the same velocity
        if (b.x_velocity == x_velocity && b.y_velocity == y_velocity) {
            add_blob(b);
            // Add of its neighbors to the list of remaning blobs to check
            for (set<long>::iterator lit=b.neighbors.begin(); lit!=b.neighbors.end(); ++lit) {
                long neighbor_id = *lit;
                if (checked.find(neighbor_id) == checked.end())
                    remaining.insert(neighbor_id);
            }
        }
        checked.insert(b_id);
    }
};

void CompositeObject::computeMask(map<long,Blob>& blob_map) {
    // Recompute the bounding box
    x_min = numeric_limits<int>::max();
    x_max = numeric_limits<int>::min();
    y_min = numeric_limits<int>::max();
    y_max = numeric_limits<int>::min();
    for (set<long>::iterator it=blob_ids.begin(); it!=blob_ids.end(); ++it) {
        long b_id = *it;
        assert(blob_map.find(b_id) != blob_map.end());
        Blob& b = blob_map[b_id];
        update_bounding_box(b);
    }

    // Create a new mask
    // TODO: Maybe better to use a re-initialize method
    mask = PixelMask(x_max - x_min + 1, y_max - y_min + 1);

    // Fill in the new mask
    for (set<long>::iterator it=blob_ids.begin(); it!=blob_ids.end(); ++it) {
        long b_id = *it;
        assert(blob_map.find(b_id) != blob_map.end());
        Blob& b = blob_map[b_id];
        for (int y=0; y<b.mask.height; ++y) {
            for (int x=0; x<b.mask.width; ++x) {
                if (b.mask.get_pixel(x, y)) {
                    mask.set_pixel(b.x_min - x_min + x, b.y_min - y_min + y, true);
                }
            }
        }
    }
};

void CompositeObject::to_string(bool verbose) {
    printf("Composite Object %ld: Size %d Velocity (%d,%d) FramesSinceLastMovement %d NumBlobs %d\n",
           id, mask.size, x_velocity, y_velocity, frames_since_last_movement, int(blob_ids.size()));
};

Prototype::Prototype() {
    id = -1;
};

Prototype::Prototype (CompositeObject& obj, long id) :
    id(id), seen_count(0), frames_since_last_seen(0),
    times_seen_this_frame(0), is_valid(false), self_likelihood(0), alpha(.2)
{
    // Add this object and save its mask
    obj_ids.insert(obj.id);
    masks.push_back(obj.mask);
};

void Prototype::get_pixel_match(const CompositeObject& obj, float& overlap, int& mask_indx) {
    float best_match = -1;
    int best_match_indx = -1;
    for (int i=0; i<masks.size(); ++i) {
        float match = masks[i].get_pixel_overlap_percent(obj.mask);
        if (match > best_match) {
            best_match = match;
            best_match_indx = i;
        }
    }
    overlap = best_match;
    mask_indx = best_match_indx;
};

void Prototype::to_string(bool verbose) {
    printf("Prototype %ld: num_obj_instances %d self_likelihood %f\n",
           id, int(obj_ids.size()), self_likelihood);

    if (verbose) {
        for (int i=0; i<masks.size(); ++i) {
            printf("Mask %d:\n", i);
            masks[i].to_string();
        }
    }
}

VisualProcessor::VisualProcessor(OSystem* _osystem, GameSettings* _game_settings) : 
    p_osystem(_osystem),
    game_settings(_game_settings),
    max_history_len(50), //numeric_limits<int>::max()),
    blob_ids(0), obj_ids(0), proto_ids(0),
    self_id(-1),
    focused_entity_id(-1), focus_level(-1), display_mode(0), display_self(false)
{
    // Get the height and width of the screen
    MediaSource& mediasrc = p_osystem->console().mediaSource();
    screen_width  = mediasrc.width();
    screen_height = mediasrc.height();

    // Load up saved self images
    loadSelfObjects();
    loadClassObjects();
  
    // Register ourselves as an event handler if a screen is present
    if (p_osystem->p_display_screen)
        p_osystem->p_display_screen->registerEventHandler(this);
};

void VisualProcessor::process_image(const IntMatrix* screen_matrix, Action action) {
    curr_blobs.clear();
    find_connected_components(*screen_matrix, curr_blobs);

    if (blob_hist.size() > 1) {
        find_blob_matches(curr_blobs);

        // Merge blobs into objects
        update_existing_objs();
        merge_blobs(curr_blobs);

        // Sanitize objects
        sanitize_objects();

        // Merge objects into classes
        //merge_objects(.96);

        // Identify which object we are
        // identify_self();
        manual_identify_self();

        // Assign objects to the save obj class files
        manual_identify_classes();
    }

    // Save State and action history
    blob_hist.push_back(curr_blobs);
    screen_hist.push_back(*screen_matrix);
    action_hist.push_back(action);
    assert(action_hist.size() == screen_hist.size());
    assert(action_hist.size() == blob_hist.size());  
    while (action_hist.size() > max_history_len) {
        action_hist.pop_front();
        screen_hist.pop_front();
        blob_hist.pop_front();
    }
};

void VisualProcessor::find_connected_components(const IntMatrix& screen_matrix, map<long,Blob>& blob_map) {
    //double start = omp_get_wtime();
    // Pointer to a swatch for each pixel
    swath* swath_mat[screen_height][screen_width];

    int num_neighbors = 4;
    int neighbors_y[] = {-1, -1, -1,  0};
    int neighbors_x[] = {-1,  0,  1, -1};
    // 1- First Scan
    int i, j, y, x, color_ind, neighbors_ind;
    for (i=0; i<screen_height; ++i) {
        for (j=0; j<screen_width; ++j) {
            swath* match1 = NULL; // Unique swatch matches
            swath* match2 = NULL; // there can be only 2 max
            color_ind = screen_matrix[i][j];
            // find the region of i,j based on west and north neighbors.
            for (neighbors_ind = 0; neighbors_ind < num_neighbors; neighbors_ind++) {
                y = i + neighbors_y[neighbors_ind];
                x = j + neighbors_x[neighbors_ind];
                if (x < 0 || x >= screen_width || y < 0 || y >= screen_height)
                    continue;
                if (screen_matrix[y][x] == color_ind) {
                    swath* match = swath_mat[y][x];
                    while (match->parent != NULL)
                        match = match->parent;

                    if (match == match1 || match == match2)
                        continue;
                    else if (match1 == NULL)
                        match1 = match;
                    else if (match2 == NULL)
                        match2 = match;
                    else
                        assert(false);
                }
            }

            if (match1 == NULL) { // This pixel is a new region
                swath* s = new swath(color_ind, j, i); // This may hit performance hard!
                s->update_bounding_box(j,i);
                swath_mat[i][j] = s;

            } else if (match2 == NULL) { // This is part of a current region
                assert(match1->parent == NULL);
                match1->x.push_back(j);
                match1->y.push_back(i);
                match1->update_bounding_box(j,i);
                swath_mat[i][j] = match1;

            } else { // Multiple matches -- merge regions
                assert(match1 != NULL && match2 != NULL);
                // Walk up parent pointers
                while (match1->parent != NULL)
                    match1 = match1->parent;
                while (match2->parent != NULL)
                    match2 = match2->parent;

                // Add new pixel to match1
                match1->x.push_back(j);
                match1->y.push_back(i);
                match1->update_bounding_box(j,i);
                swath_mat[i][j] = match1;

                if (match1 != match2) {
                    match2->parent = match1;
                    match1->update_bounding_box(*match2);
                }
            }
        }
    }

    // Convert swaths into blobs
    long blob_mat[screen_height][screen_width];
    for (int y=0; y<screen_height; ++y) {
        for (int x=0; x<screen_width; ++x) {
            blob_mat[y][x] = -1;
        }
    }
    map<swath*,long> swath_map;
    for (int y=0; y<screen_height; ++y) {
        for (int x=0; x<screen_width; ++x) {
            swath* s = swath_mat[y][x];
            if (swath_map.find(s) != swath_map.end())
                continue;

            // Check if some parent of this swath has been blobified
            long blob_parent_id = -1;
            swath *p = s;
            while (p->parent != NULL) {
                p = p->parent;
                if (swath_map.find(p) != swath_map.end()) {
                    blob_parent_id = swath_map[p];
                    break;
                }
            }

            // If no blob parent is found, create a new blob
            if (blob_parent_id == -1) {
                Blob b(p->color,blob_ids++,p->x_min,p->x_max,p->y_min,p->y_max);
                // Add all of s' pixels to b as well as s' parent's pixels
                do {
                    for (int i=0; i<s->x.size(); ++i) {
                        b.mask.set_pixel(s->x[i] - b.x_min, s->y[i] - b.y_min, true);
                        blob_mat[s->y[i]][s->x[i]] = b.id;
                    }
                    swath_map[s] = b.id;
                    s = s->parent;
                } while (s != NULL);
                blob_map[b.id] = b;

            } else { // A blob has already been created. Add to it
                Blob& b = blob_map[blob_parent_id];
                do {
                    for (int i=0; i<s->x.size(); ++i) {
                        b.mask.set_pixel(s->x[i] - b.x_min, s->y[i] - b.y_min, true);
                        blob_mat[s->y[i]][s->x[i]] = b.id;
                    }
                    swath_map[s] = b.id;
                    s = s->parent;
                } while (s != p);
            }
        }
    }

    // Delete the swaths
    for (map<swath*,long>::iterator it=swath_map.begin(); it!=swath_map.end(); ++it) {
        delete it->first;
    }

    // Populate neighbors
    for (int i = 0; i < screen_height; i++) {
        for (int j = 0; j < screen_width; j++) {
            long bid = blob_mat[i][j];
            if (j+1 < screen_width && bid != blob_mat[i][j+1]) {
                blob_map[bid].add_neighbor(blob_mat[i][j+1]);
                blob_map[blob_mat[i][j+1]].add_neighbor(bid);
            }
            if (i+1 < screen_height && bid != blob_mat[i+1][j]) {
                blob_map[bid].add_neighbor(blob_mat[i+1][j]);
                blob_map[blob_mat[i+1][j]].add_neighbor(bid);
            }
        }
    }

    // double end = omp_get_wtime();
    // cout << "Blob Detection: " << end-start << endl;
};


// Try to match all of the current blobs with the equivalent blobs from the last timestep
void VisualProcessor::find_blob_matches(map<long,Blob>& blobs) {
#ifdef MUNKRES
    // Solve blob matching via Hungarian algorithm. Better matches but more time.
    map<long,Blob>& old_blobs = blob_hist.back();
    int width = max(old_blobs.size(), blobs.size());
    Matrix<double> matrix(width, width);
    int old_blob_cnt = 0;
    for (map<long,Blob>::iterator it=old_blobs.begin(); it!=old_blobs.end(); ++it) {
        Blob& old_blob = it->second;
        int new_blob_cnt = 0;
        for(map<long,Blob>::iterator it2=blobs.begin(); it2!=blobs.end(); it2++) {
            Blob& new_blob = it2->second;
            float match = new_blob.get_aggregate_blob_match(old_blob);
            matrix(old_blob_cnt,new_blob_cnt) = (double) (1.0 - match);
            new_blob_cnt++;
        }
        while (new_blob_cnt < width) {
            matrix(old_blob_cnt,new_blob_cnt) = (double) 1.0;
            new_blob_cnt++;
        }
        old_blob_cnt++;
    }
    while (old_blob_cnt < width) {
        for (int i=0; i<width; ++i)
            matrix(old_blob_cnt,i) = (double) 1.0;
        old_blob_cnt++;
    }

    Munkres m;
    m.solve(matrix);

    old_blob_cnt = 0;
    for (map<long,Blob>::iterator it=old_blobs.begin(); it!=old_blobs.end(); ++it) {
        Blob& old_blob = it->second;
        int new_blob_cnt = 0;
        for(map<long,Blob>::iterator it2=blobs.begin(); it2!=blobs.end(); it2++) {
            Blob& new_blob = it2->second;
            if (matrix(old_blob_cnt,new_blob_cnt) == 0) {
                new_blob.compute_velocity(old_blob);
                new_blob.parent_id = old_blob.id;
                old_blob.child_id = new_blob.id;
            }
            new_blob_cnt++;
        }
        old_blob_cnt++;
    }
#else
    // Do greedy (fast) blob matching
    map<long,Blob>& old_blobs = blob_hist.back();
    for (map<long,Blob>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
        Blob& b = it->second;
        long blob_match_id = b.find_matching_blob(old_blobs);
        if (blob_match_id < 0) continue;
        assert(old_blobs.find(blob_match_id) != old_blobs.end());
        Blob& match = old_blobs[blob_match_id];
        b.compute_velocity(match);
        b.parent_id = match.id;
        match.child_id = b.id;
    }
#endif
};

// Update the current blobs that we already have
void VisualProcessor::update_existing_objs() {
    set<long> used_blob_ids; // A blob becomes used when it is integrated into an existing object
    set<long> new_blob_ids; // Blobs who have children in the current timestep
    vector<long> to_remove;
    map<long,Blob>& old_blobs = blob_hist.back();

    for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
        CompositeObject& obj = it->second;
        obj.age++;
        new_blob_ids.clear();
    
        // Update the object's blobs to their equivalents in the next frame
        for (set<long>::iterator bit=obj.blob_ids.begin(); bit!=obj.blob_ids.end(); bit++) {
            long b_id = *bit;
            assert(old_blobs.find(b_id) != old_blobs.end());
            Blob& b = old_blobs[b_id];

            // If b has a valid child and we havent already allocated the child
            if (b.child_id >= 0 && used_blob_ids.find(b.child_id) == used_blob_ids.end()) {
                new_blob_ids.insert(b.child_id);
            }
        }

        // If no new blobs were found for this object, remove it
        if (new_blob_ids.empty()) {
            to_remove.push_back(obj.id);
            // if (obj.id == self_id) {
            //     printf("Removing self object due to no new blobs.\n");
            //     focused_obj_id=obj.id;
            //     IntMatrix screen_cpy(screen_hist.back());
            //     display_screen(screen_cpy);
            //     p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
            //     cin.get();
            // }
            continue;
        }

        // Checks that the new blobs are velocity consistent
        long first_id = *(new_blob_ids.begin());
        Blob& first = curr_blobs[first_id];
        bool velocity_consistent = true;
        for (set<long>::iterator bit=new_blob_ids.begin(); bit!=new_blob_ids.end(); ++bit) {
            long b_id = *bit;
            assert(curr_blobs.find(b_id) != curr_blobs.end());      
            Blob& b = curr_blobs[b_id];
            // Velocity check
            if (b.x_velocity != first.x_velocity || b.y_velocity != first.y_velocity) {
                velocity_consistent = false;
                // point centroid = obj.get_centroid();
                // printf("Found inconsistent velocitied object loc %d %d\n",centroid.x,centroid.y);
                // printf("first blob velocity %d %d, curr blob velocity %d %d\n",first.x_velocity,first.y_velocity,b.x_velocity,b.y_velocity);
                // focused_obj_id=obj.id;
                // IntMatrix screen_cpy(screen_hist.back());
                // display_screen(screen_cpy);
                // p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
                // cin.get();
                break;
            } 
        }

        // Update the object with new blobs
        if (velocity_consistent) {
            obj.clear();
            obj.x_velocity = first.x_velocity;
            obj.y_velocity = first.y_velocity;
            for (set<long>::iterator bit=new_blob_ids.begin(); bit!=new_blob_ids.end(); ++bit) {
                long b_id = *bit;
                assert(curr_blobs.find(b_id) != curr_blobs.end());      
                Blob& b = curr_blobs[b_id];
                obj.add_blob(b);
            }
            // Expand the object if it is non-stationary and has a change in bounding box
            if (obj.x_velocity != 0 || obj.y_velocity != 0)
                obj.expand(curr_blobs);
            obj.computeMask(curr_blobs);
            used_blob_ids.insert(obj.blob_ids.begin(), obj.blob_ids.end());
        } else {
            // This object is no longer legitimate. Decide what to do with it.
            to_remove.push_back(obj.id);
            // if (obj.id == self_id) {
            //     printf("Removing self object due to velocity inconcistencies.\n");
            //     focused_obj_id=obj.id;
            //     IntMatrix screen_cpy(screen_hist.back());
            //     display_screen(screen_cpy);
            //     p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
            //     cin.get();
            // }
            continue;
        }
    }

    // Walk through list in reverse removing objects
    for (int i=0; i<to_remove.size(); ++i) {
        assert(composite_objs.find(to_remove[i]) != composite_objs.end());
        composite_objs.erase(to_remove[i]); 
    }
};

// Merge equivalent blobs into a single composite object
void VisualProcessor::merge_blobs(map<long,Blob>& blobs) {
    set<long> checked;
  
    // Add all the blobs in the existing objects to the list of checked blobs
    for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
        CompositeObject& obj = it->second;
        checked.insert(obj.blob_ids.begin(), obj.blob_ids.end());
    }

    // Now create objects from any blobs which aren't already accounted for
    for (map<long,Blob>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
        Blob& b = it->second;

        if (checked.find(b.id) != checked.end())
            continue;
    
        // Only create blobs when velocity is greater than zero
        if (b.x_velocity != 0 || b.y_velocity != 0) {
            CompositeObject obj(b.x_velocity, b.y_velocity, obj_ids++);
            obj.add_blob(b);
            obj.expand(blobs); //TODO: This expand could use blobs in use by other objects....
            obj.computeMask(blobs);
            composite_objs[obj.id] = obj;
            for (set<long>::iterator bit=obj.blob_ids.begin(); bit!=obj.blob_ids.end(); ++bit) {
                assert(checked.find(*bit) == checked.end());
            }
            checked.insert(obj.blob_ids.begin(), obj.blob_ids.end());
        }
        checked.insert(b.id);
    }
};

void VisualProcessor::sanitize_objects() {
    vector<long> to_remove;

    for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
        CompositeObject& obj = it->second;

        // (piyushk) if blob is too small or the velocity is 0, then remove the object
        if (obj.mask.size < 15) {
            to_remove.push_back(obj.id);
            continue;
        }
        if (obj.frames_since_last_movement > 50) {
            to_remove.push_back(obj.id);
            continue;
        }
        if (obj.x_velocity == 0 && obj.y_velocity == 0) {
            obj.frames_since_last_movement++;      
        } else {
            obj.frames_since_last_movement = 0;
        }
    }

    // Walk through list in reverse removing objects
    for (int i=0; i<to_remove.size(); ++i) {
        assert(composite_objs.find(to_remove[i]) != composite_objs.end());
        composite_objs.erase(to_remove[i]); 
    }
}

// Identify self based on prototypes who have only one instance
void VisualProcessor::identify_self() {
    vector<Prototype*> singles;
    float decay = .995;

    for (int i=0; i<obj_classes.size(); i++) {
        Prototype& p = obj_classes[i];

        if (p.obj_ids.size() == 0)
            continue;

        if (p.obj_ids.size() == 1) {
            singles.push_back(&p);
        } else if (p.obj_ids.size() > 1) { // Update to target of zero
            p.self_likelihood -= p.alpha * p.self_likelihood;
            p.alpha *= decay;

            // if (p.id == 6) {
            //     printf("Decreasing likelihood of prototype %d to %f\n",p.id,p.self_likelihood);
            //     IntMatrix screen_cpy(screen_hist.back());
            //     display_screen(screen_cpy);
            //     for (set<long>::iterator it=p.obj_ids.begin(); it!=p.obj_ids.end(); it++) {
            //         long obj_id = *it;
            //         assert(composite_objs.find(obj_id) != composite_objs.end());
            //         box_object(composite_objs[obj_id],screen_cpy,258);
            //     }
            //     p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
            //     cin.get();
            // }
        }
    }

    // Ideal case -- increase self likelihood
    if (singles.size() == 1) {
        Prototype* p = singles[0];
        p->self_likelihood += p->alpha * (1.0f - p->self_likelihood);
        p->alpha *= decay;

        // printf("Increasing likelihood of prototype %d to %f\n",p->id,p->self_likelihood);
        // IntMatrix screen_cpy(screen_hist.back());
        // display_screen(screen_cpy);
        // box_object(composite_objs[*(p->obj_ids.begin())],screen_cpy,256);
        // p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
        // cin.get();
    } else if (singles.size() > 1) { // Multiple singles... Decrease self likelihood of each
        // printf("Multiple prototypes ");
        // IntMatrix screen_cpy(screen_hist.back());
        // display_screen(screen_cpy);

        // for (int i=0; i<singles.size(); i++) {
        //     Prototype* p = singles[i];
        //     printf("%d likelihood %f, ",p->id, p->self_likelihood);
        //     box_object(composite_objs[*(p->obj_ids.begin())],screen_cpy,260);
        // }
        // printf("\n");
        // p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
        // cin.get();
    }
};

void VisualProcessor::manual_identify_self() {
    // Identify the self based on manually loaded self file
    for (map<long,CompositeObject>::iterator it=composite_objs.begin();
         it!=composite_objs.end(); it++) {
        CompositeObject& obj = it->second;
        for (int i=0; i<manual_self_objects.size(); ++i) {
            if (manual_self_objects[i].mask.equals(obj.mask)) {
                self_id = obj.id;
                break;
            }
        }
    }
};

void VisualProcessor::manual_identify_classes() {
    // Sort objects into classes based on manually identified classes
    for (int i=0; i<manual_obj_classes.size(); ++i) {
        manual_obj_classes[i].obj_ids.clear();
    }
    for (map<long,CompositeObject>::iterator it=composite_objs.begin();
         it!=composite_objs.end(); it++) {
        CompositeObject& obj = it->second;

        // Try to assign this object to one of the object classes
        for (int i=0; i<manual_obj_classes.size(); ++i) {
            Prototype& proto = manual_obj_classes[i];
            for (int j=0; j<proto.masks.size(); ++j) {
                if (proto.masks[i].equals(obj.mask)) {
                    proto.obj_ids.insert(obj.id);
                    break;
                }
            }
        }
    }
}


// void VisualProcessor::identify_self() {
//     float max_info_gain = -1;
//     long best_blob_id = -1;
//     for (map<long,Blob>::iterator it=curr_blobs.begin(); it!=curr_blobs.end(); ++it) {
//         long b_id = it->first;

//         int blob_history_len = 0;
//         vector<pair<int,int> > velocity_hist;
//         map<pair<int,int>,int> velocity_counts;

//         assert(curr_blobs.find(b_id) != curr_blobs.end());
//         Blob* b = &curr_blobs[b_id];

//         // Get the velocity history of this blob
//         while (b->parent_id >= 0 && blob_history_len < max_history_len) {
//             // Push back the velocity
//             pair<int,int> vel(b->x_velocity, b->y_velocity);
//             velocity_hist.push_back(vel);
//             velocity_counts[vel] = GetWithDef(velocity_counts,vel,0) + 1;

//             blob_history_len++;

//             // Get the parent
//             map<long,Blob>& old_blobs = blob_hist[blob_hist.size() - blob_history_len];
//             long parent_id = b->parent_id;
//             assert(old_blobs.find(parent_id) != old_blobs.end());
//             b = &old_blobs[parent_id];
//         }

//         // How many times was each action performed?
//         map<Action,int> action_counts;
//         vector<Action> act_vec;
//         for (int i=0; i<blob_history_len; ++i) {
//             Action a = action_hist[action_hist.size()-i-1];
//             act_vec.push_back(a);
//             action_counts[a] = GetWithDef(action_counts,a,0) + 1;
//         }

//         assert(act_vec.size() == velocity_hist.size());

//         // Calculate H(velocities)
//         float velocity_entropy = compute_entropy(velocity_counts,blob_history_len);

//         // Calculate H(velocity|a)
//         float action_entropy = 0;
//         for (map<Action,int>::iterator it2=action_counts.begin(); it2!=action_counts.end(); ++it2) {
//             Action a = it2->first;
//             int count = it2->second;
//             float p_a = count / (float) blob_history_len;
//             map<pair<int,int>,int> selective_counts;
//             int selective_total = 0;
//             for (int i=0; i<blob_history_len; ++i) {
//                 if (act_vec[i] == a) {
//                     pair<int,int> vel = velocity_hist[i];
//                     selective_counts[vel] = GetWithDef(selective_counts,vel,0) + 1;
//                     selective_total++;
//                 }
//             }
//             float selective_entropy = compute_entropy(selective_counts,selective_total);
//             action_entropy += p_a * selective_entropy;
//         }

//         float info_gain = velocity_entropy - action_entropy;
//         if (info_gain > max_info_gain) {
//             max_info_gain = info_gain;
//             best_blob_id = b_id;
//         }
//     }
//     //printf("Max info gain: %f\n",max_info_gain);
//     //best_blob->to_string();
//     self_id = best_blob_id;
// };

point VisualProcessor::get_self_centroid() {
    if (composite_objs.find(self_id) == composite_objs.end()) {
        return point(-1,-1);
    }
    return composite_objs[self_id].get_centroid();
};

bool VisualProcessor::found_self() {
    return composite_objs.find(self_id) != composite_objs.end();
}

// Merges together objects into classes of objects
void VisualProcessor::merge_objects(float similarity_threshold) {
    set<long> checked_objs; // Objects found to match a prototype

    // Check which of the prototype's objects are still valid
    for (int i=0; i<obj_classes.size(); ++i) {
        Prototype& p = obj_classes[i];
        p.times_seen_this_frame = 0; //(piyushk)
        set<long> to_erase;

        for (set<long>::iterator it=p.obj_ids.begin(); it!=p.obj_ids.end(); it++) {
            long obj_id = *it;

            // If the obj no longer exists, remove it
            if (composite_objs.find(obj_id) == composite_objs.end()) {
                to_erase.insert(obj_id);
                continue;
            }

            // Check to make sure that object still matches the prototype
            CompositeObject& obj = composite_objs[obj_id];
            float pixel_similarity = 0; int best_indx = 0; // TODO: Only need to do this if obj changed
            p.get_pixel_match(obj, pixel_similarity, best_indx);
            if (pixel_similarity >= similarity_threshold) {
                checked_objs.insert(obj.id);
                p.times_seen_this_frame++; //(piyushk)
            } else {
                to_erase.insert(obj_id);
            }
        }

        // Erase all the bad ids
        for (set<long>::iterator it=to_erase.begin(); it!=to_erase.end(); ++it) {
            long id = *it;
            p.obj_ids.erase(id);
        }
    }

    // Create new prototypes for novel objects
    for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
        if (checked_objs.find(it->first) != checked_objs.end())
            continue; 

        // See if any of the existing prototypes match this object
        CompositeObject& obj = it->second;
        bool found_match = false;
        for (int j=0; j<obj_classes.size(); ++j) {
            Prototype& p = obj_classes[j];
            float pixel_similarity = 0; int best_indx = 0;
            p.get_pixel_match(obj, pixel_similarity, best_indx);
            if (pixel_similarity > similarity_threshold) {
                p.obj_ids.insert(obj.id);
                p.times_seen_this_frame++; //(piyushk)
                found_match = true;
                break;
            }
        }
   
        // Insert new prototype
        if (!found_match) {
            Prototype p(obj, proto_ids++);
            obj_classes.push_back(p);
        }
    }

    //(piyushk) remove bad prototypes
    vector<int> prototypes_to_erase;
    for (int i=0; i<obj_classes.size(); ++i) {
        Prototype& p = obj_classes[i];
        if (!p.times_seen_this_frame)
            p.frames_since_last_seen++;
        else
            p.frames_since_last_seen = 0;
        if (p.seen_count < 25 && p.frames_since_last_seen > 3) {
            prototypes_to_erase.push_back(i);
        } else if (p.seen_count >= 25 && !p.is_valid) {
            p.is_valid = true;
        }
        p.seen_count += p.times_seen_this_frame;
    }
    for (int i = prototypes_to_erase.size() - 1; i >= 0; --i) {
        obj_classes.erase(obj_classes.begin() + prototypes_to_erase[i]);
    }
};

void VisualProcessor::printVelHistory(CompositeObject& obj) {
    for (set<long>::iterator it=obj.blob_ids.begin(); it!=obj.blob_ids.end(); ++it) {
        long b_id = *it;
    
        assert(curr_blobs.find(b_id) != curr_blobs.end());
        Blob* b = &curr_blobs[b_id];
        printf("Blob %ld: ",b_id);
        // Get the velocity history of this blob
        int blob_history_len = 1;
        while (b->parent_id >= 0 && blob_history_len < max_history_len) {
            // Push back the velocity
            pair<int,int> vel(b->x_velocity, b->y_velocity);
            blob_history_len++;
            string action_name = action_to_string(action_hist[action_hist.size() - blob_history_len]);
            printf("%s (%d,%d)\n",action_name.c_str(), b->x_velocity, b->y_velocity);
            // Get the parent
            map<long,Blob>& old_blobs = blob_hist[blob_hist.size() - blob_history_len];
            long parent_id = b->parent_id;
            assert(old_blobs.find(parent_id) != old_blobs.end());
            b = &old_blobs[parent_id];
        }
        printf("\n");
    }  
};

void VisualProcessor::exportMask(int width, int height, vector<char>& mask, const string& filename) {
    ofstream out;
    
    try {
        out.open(filename.c_str(), ios_base::binary);
        if(!out)
            throw "Couldn't open PNG file";

        // Write the width and height to file
        out.write((char *)(&width), sizeof(width));
        out.write((char *)(&height), sizeof(height));

        // Write the file mask
        for (int i=0; i<mask.size(); ++i)
            out.put(mask[i]);
        
        out.close();
    }
    catch(const char *msg)
    {
        out.close();
        cerr << msg << endl;
    }
};

void VisualProcessor::importMask(int& width, int& height, vector<char>& mask, int& size,
                                 const string& filename) {
    ifstream fin;
    size = 0;
    try {
        fin.open(filename.c_str(), ios_base::binary);
        if(!fin)
            throw "Couldn't open file";

        mask.clear();
        
        // Write the width and height to file
        fin.read((char *)(&width), sizeof(width));
        fin.read((char *)(&height), sizeof(height));

        // Read the file mask
        while (1) {
            char c;
            fin.get(c);
            if (fin.eof())
                break;
            mask.push_back(c);
            size += count_ones(c);
        }
        
        fin.close();
    }
    catch(const char *msg)
    {
        fin.close();
        cerr << msg << endl;
    }
};

void VisualProcessor::saveSelection() {
    return;
    // if (focus_level != 1 && focus_level != 2) {
    //     printf("First select and object or object class by pressing \'w\' or \'e\' and clicking an object.\n");
    //     return;
    // }

    // string image_prefix, image_suffix;
    // int width, height;
    // vector<char>* p_mask;

    // if (focus_level == 1) { // Saving a self object
    //     assert(composite_objs.find(focused_entity_id) != composite_objs.end());
    //     CompositeObject& obj = composite_objs[focused_entity_id];
    //     obj.computeMask(curr_blobs); // Recompute the object's mask just to be sure
    //     width = obj.mask.width; height = obj.mask.height;
    //     p_mask = &obj.mask;
    //     image_prefix = SELF_IMAGE_PREFIX;
    //     image_suffix = SELF_IMAGE_SUFFIX;
    // } else if (focus_level == 2) { // Saving an object class
    //     // Find the selected prototype
    //     int protoIndx = -1;
    //     for (int i=0; i<obj_classes.size(); ++i) {
    //         if (obj_classes[i].id == focused_entity_id) {
    //             protoIndx = i;
    //             break;
    //         }
    //     }
    //     assert(protoIndx >= 0);
    //     Prototype& proto = obj_classes[protoIndx];
    //     width = proto.width; height = proto.height;
    //     p_mask = &proto.mask;
    //     image_prefix = CLASS_IMAGE_PREFIX;
    //     image_suffix = CLASS_IMAGE_SUFFIX;
    // }

    // using namespace boost::filesystem;

    // // Take care of the directory creation
    // path p(IMAGE_FILENAME);
    // string rom_name = game_settings->s_rom_file;
    // p /= rom_name;
    // if (!exists(p) || !is_directory(p))
    //     create_directories(p);

    // // Select which file to write to
    // for (int i=1; ; i++) {
    //     string filename = image_prefix + boost::lexical_cast<std::string>(i) + image_suffix;
    //     p /= filename;
    //     if (exists(p) && is_regular_file(p)) {
    //         // Import each previously saved object 
    //         int saved_width, saved_height, saved_size;
    //         vector<char> saved_mask;
    //         importMask(saved_width, saved_height, saved_mask, saved_size, p.string());
    //         if (maskEquals(width, height, *p_mask, saved_width, saved_height, saved_mask)) {
    //             printf("Matching object was already saved in file %s.\n",p.string().c_str());
    //             return;
    //         }                
    //         p = p.parent_path();
    //     }
    //     else
    //         break;
    // }
    // printf("Exporting selection to %s.\n",p.string().c_str());
    // exportMask(width, height, *p_mask, p.string());
}

void VisualProcessor::loadClassObjects() {
    return;
    // using namespace boost::filesystem;
    // path p(IMAGE_FILENAME);
    // string rom_name = game_settings->s_rom_file;
    // p /= rom_name;

    // for (int i=1; ; i++) {
    //     string filename = CLASS_IMAGE_PREFIX + boost::lexical_cast<std::string>(i) + CLASS_IMAGE_SUFFIX;
    //     p /= filename;
    //     if (exists(p) && is_regular_file(p)) {
    //         CompositeObject obj;
    //         importMask(obj.width, obj.height, obj.mask, obj.size, p.string());
    //         printf("Loading class-image: %s ...",p.string().c_str());
    //         Prototype proto;
    //         proto.width = obj.width;
    //         proto.height = obj.height;
    //         proto.mask = obj.mask;
    //         proto.size = obj.size;

    //         // Check to make sure there isn't a duplicate object class already in memory
    //         bool found_match = false;
    //         for (int j=0; j<manual_obj_classes.size(); ++j) {
    //             Prototype& p = manual_obj_classes[i];
    //             if (maskEquals(p.width, p.height, p.mask, proto.width, proto.height, proto.mask)) {
    //                 found_match = true;
    //                 break;
    //             }
    //         }

    //         if (!found_match) {
    //             manual_obj_classes.push_back(proto);
    //             printf(" successful!\n");
    //         } else {
    //             printf(" cancelled due to existing duplicate.\n");
    //         }

    //         p = p.parent_path();
    //     } else
    //         break;
    // }
};

void VisualProcessor::loadSelfObjects() {
    return;
    // using namespace boost::filesystem;
    // path p(IMAGE_FILENAME);
    // string rom_name = game_settings->s_rom_file;
    // p /= rom_name;

    // // Select which file to write to
    // for (int i=1; ; i++) {
    //     string filename = SELF_IMAGE_PREFIX + boost::lexical_cast<std::string>(i) + SELF_IMAGE_SUFFIX;
    //     p /= filename;
    //     if (exists(p) && is_regular_file(p)) {
    //         CompositeObject obj;
    //         importMask(obj.width, obj.height, obj.mask, obj.size, p.string());
    //         printf("Loading self-image: %s ...",p.string().c_str());

    //         // Check to make sure there isn't a duplicate self object already in memory
    //         bool found_match = false;
    //         for (int j=0; j<manual_self_objects.size(); ++j) {
    //             if (manual_self_objects[j].maskEquals(obj)) {
    //                 found_match = true;
    //                 break;
    //             }
    //         }

    //         if (!found_match) {
    //             manual_self_objects.push_back(obj);
    //             printf(" successful!\n");
    //         } else {
    //             printf(" cancelled due to existing duplicate.\n");
    //         }

    //         p = p.parent_path();
    //     } else
    //         break;
    // }
}

void VisualProcessor::handleSDLEvent(const SDL_Event& event) {
    switch(event.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == 1) {
            int sdl_screen_width = p_osystem->p_display_screen->screen->w;
            int sdl_screen_height = p_osystem->p_display_screen->screen->h;
            int approx_x = (screen_width * event.button.x) / sdl_screen_width;
            int approx_y = (screen_height * event.button.y) / sdl_screen_height;

            focused_entity_id = -1;
            // Look for an object that falls under these coordinates
            if (focus_level < 0)
                break;
            else if (focus_level == 0) {
                // Find a blob that is under the click
                for (map<long,Blob>::iterator it=curr_blobs.begin(); it != curr_blobs.end(); it++) {
                    Blob& b = it->second;
                    if (approx_x >= b.x_min && approx_x <= b.x_max &&
                        approx_y >= b.y_min && approx_y <= b.y_max) {
                        // Check to see if the blob contains the actual pixel
                        bool has_pixel = b.mask.get_pixel(approx_x - b.x_min, approx_y - b.y_min);
                        if (has_pixel) {
                            focused_entity_id = b.id;
                            b.to_string();
                            break;
                        }
                    }
                }
            } else if (focus_level == 1 || focus_level == 2) {
                // Find an object that is under the click
                for (map<long,CompositeObject>::iterator it=composite_objs.begin();
                     it!=composite_objs.end(); ++it) {
                    CompositeObject& obj = it->second;
                    if (approx_x >= obj.x_min && approx_x <= obj.x_max &&
                        approx_y >= obj.y_min && approx_y <= obj.y_max) {
                        focused_entity_id = obj.id;
                        if (focus_level == 1) obj.to_string();
                        break;
                    }
                }
                // To which prototype does this obj belong?
                if (focus_level == 2 && focused_entity_id > 0) { 
                    long obj_id = focused_entity_id;
                    focused_entity_id = -1;
                    for (int i=0; i<obj_classes.size(); i++) {
                        Prototype& p = obj_classes[i];
                        if (p.obj_ids.find(obj_id) != p.obj_ids.end()) {
                            focused_entity_id = p.id;
                            p.to_string();
                            break;
                        }
                    }
                }
            } else {
                printf("Unexpected focus level: %d. Not sure what type of object to display.\n",
                       focus_level);
            }
            // TODO: Consolidate all of these display screen calls
            // Update the screen
            if (screen_hist.size() >= 1) {
                IntMatrix screen_cpy(screen_hist.back());
                display_screen(screen_cpy);
                p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(),
                                                            screen_cpy.size());
            }
        }
        break;

    case SDL_KEYDOWN:
        switch(event.key.keysym.sym) {
        case SDLK_0:
            display_mode = 0;
            break;
        case SDLK_1:
            display_mode = display_mode == 1 ? 0 : 1;
            break;
        case SDLK_2:
            display_mode = display_mode == 2 ? 0 : 2;
            break;
        case SDLK_3:
            display_mode = display_mode == 3 ? 0 : 3;
            break;
        case SDLK_4:
            display_self = !display_self;
            break;
        case SDLK_s: // Saves the mask for the current selection
            saveSelection();
            break;
        case SDLK_q:
            if (focus_level == 0) {
                focus_level = -1;
            } else {
                focus_level = 0;
                printf("Focusing on Blobs.\n");
            }
            break;
        case SDLK_w:
            if (focus_level == 1) {
                focus_level = -1;
            } else {
                focus_level = 1;
                printf("Focusing on Objects.\n");
            }
            break;
        case SDLK_e:
            if (focus_level == 2) {
                focus_level = -1;
            } else {
                focus_level = 2;
                printf("Focusing on Prototypes.\n");
            }
            break;
        case SDLK_i: // Get "Info" about the currently selected object/blob/prototype
            if (focus_level < 0 || focused_entity_id < 0)
                break;
            if (focus_level == 0) {
                if (curr_blobs.find(focused_entity_id) != curr_blobs.end())
                    curr_blobs[focused_entity_id].to_string(true,&blob_hist);
            } else if (focus_level == 1) {
                if (composite_objs.find(focused_entity_id) != composite_objs.end())
                    composite_objs[focused_entity_id].to_string(true);
            } else if (focus_level == 2) {
                for (int i=0; i<obj_classes.size(); i++) {
                    if (obj_classes[i].id == focused_entity_id)
                        obj_classes[i].to_string(true);
                }
            }
            break;
        default:
            break;
        }
        // Update the screen
        if (screen_hist.size() >= 1) {
            IntMatrix screen_cpy(screen_hist.back());
            display_screen(screen_cpy);
            p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
        }
        break;
    
    case SDL_VIDEORESIZE:
        // Update the screen
        if (screen_hist.size() >= 1) {
            IntMatrix screen_cpy(screen_hist.back());
            display_screen(screen_cpy);
            p_osystem->p_display_screen->display_screen(screen_cpy, screen_cpy[0].size(), screen_cpy.size());
        }
        break;

    default:
        break;
    }
    // Pass the event to the superclass
    //PlayerAgent::handleSDLEvent(event);
};

// Overrides the normal display screen method to alter our display
void VisualProcessor::display_screen(IntMatrix& screen_cpy) {
    switch (display_mode) {
    case 1:
        plot_blobs(screen_cpy);
        break;
    case 2:
        plot_objects(screen_cpy);
        break;
    case 3:
        plot_prototypes(screen_cpy);
        break;
    default:
        break;
    }

    if (display_self)
        plot_self(screen_cpy);
  
    // Display focused entity
    if (focus_level >= 0 && focused_entity_id >= 0) {
        // TODO: make the blob tracking continue through frames via the child blobs
        if (focus_level == 0 && curr_blobs.find(focused_entity_id) != curr_blobs.end()) {
            Blob& b = curr_blobs[focused_entity_id];
            box_blob(b,screen_cpy,258);
        } else if (focus_level == 1 && composite_objs.find(focused_entity_id) != composite_objs.end()) {
            CompositeObject& obj = composite_objs[focused_entity_id];
            box_object(obj,screen_cpy,256);
        } else if (focus_level == 2) {
            for (int i=0; i<obj_classes.size(); i++) {
                Prototype& p = obj_classes[i];
                if (p.id == focused_entity_id) {
                    for (set<long>::iterator it=p.obj_ids.begin(); it!=p.obj_ids.end(); it++) {
                        long obj_id = *it;
                        assert(composite_objs.find(obj_id) != composite_objs.end());
                        CompositeObject& obj = composite_objs[obj_id];
                        box_object(obj,screen_cpy,260);
                    }
                    break;
                }
            }
        }
    }
};

void VisualProcessor::plot_blobs(IntMatrix& screen_matrix) {
    int region_color = 257;
    for (map<long,Blob>::iterator it=curr_blobs.begin(); it!=curr_blobs.end(); ++it) {
        Blob& b = it->second;
        region_color++;
        for (int y=0; y<b.mask.height; ++y) {
            for (int x=0; x<b.mask.width; ++x) {
                if (b.mask.get_pixel(x, y))
                    screen_matrix[b.y_min+y][b.x_min+x] = region_color;
            }
        }
    }
};

void VisualProcessor::plot_objects(IntMatrix& screen_matrix) {
    // Set the color for the bg
    for (int y=0; y<screen_height; ++y) {
        for (int x=0; x<screen_width; ++x) {
            screen_matrix[y][x] = 0;
        }
    }

    int region_color = 256;
    for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
        region_color++;
        CompositeObject& o = it->second;
        for (int y=0; y<o.mask.height; ++y) {
            for (int x=0; x<o.mask.width; ++x) {
                if (o.mask.get_pixel(x, y))
                    screen_matrix[o.y_min+y][o.x_min+x] = region_color;
            }
        }
    }
};

// Draws a box around the an object
void VisualProcessor::box_object(CompositeObject& obj, IntMatrix& screen_matrix, int color) {
    for (int x=obj.x_min; x<=obj.x_max; ++x) {
        screen_matrix[obj.y_min-1][x] = color;
        screen_matrix[obj.y_max+1][x] = color;
    }
    for (int y=obj.y_min; y<=obj.y_max; ++y) {
        screen_matrix[y][obj.x_min-1] = color;
        screen_matrix[y][obj.x_max+1] = color;
    }
};

// Draws a box around a blob
void VisualProcessor::box_blob(Blob& b, IntMatrix& screen_matrix, int color) {
    for (int x=b.x_min; x<=b.x_max; ++x) {
        if (b.y_min > 0) screen_matrix[b.y_min-1][x] = color;
        if (b.y_max < screen_height-1) screen_matrix[b.y_max+1][x] = color;
    }
    for (int y=b.y_min; y<=b.y_max; ++y) {
        if (b.x_min > 0) screen_matrix[y][b.x_min-1] = color;
        if (b.x_max < screen_width-1) screen_matrix[y][b.x_max+1] = color;
    }
};

void VisualProcessor::plot_prototypes(IntMatrix& screen_matrix) {
    // Set the color for the bg
    for (int y=0; y<screen_height; ++y) {
        for (int x=0; x<screen_width; ++x) {
            screen_matrix[y][x] = 0;
        }
    }

    int region_color = 258;
    for (int i=0; i<obj_classes.size(); ++i) {
        Prototype& p = obj_classes[i];
        if (!p.is_valid)
            continue;         // Do not display weak prototypes
        region_color++;
        for (set<long>::iterator obj_it=p.obj_ids.begin(); obj_it!=p.obj_ids.end(); ++obj_it) {
            long o_id = *obj_it;
            assert(composite_objs.find(o_id) != composite_objs.end());
            CompositeObject& o = composite_objs[o_id];
            for (int y=0; y<o.mask.height; ++y) {
                for (int x=0; x<o.mask.width; ++x) {
                    if (o.mask.get_pixel(x, y))
                        screen_matrix[o.y_min+y][o.x_min+x] = region_color;
                }
            }
        }
    }
};

void VisualProcessor::plot_self(IntMatrix& screen_matrix) {
    int self_color = 258;
    if (composite_objs.find(self_id) != composite_objs.end()) {
      CompositeObject& o = composite_objs[self_id];
      box_object(o,screen_matrix,self_color);
    }
    // if (curr_blobs.find(self_id) != curr_blobs.end()) {
    //     Blob& b = curr_blobs[self_id];
    //     for (int y=0; y<b.height; ++y) {
    //         for (int x=0; x<b.width; ++x) {
    //             if (get_pixel(b.width, b.height, x, y, b.mask))
    //                 screen_matrix[b.y_min+y][b.x_min+x] = self_color;
    //         }
    //     }
    // }
};
