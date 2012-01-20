/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 **************************************************************************** */

#include "self_detection_agent.h"
#include "random_tools.h"
#include "System.hxx"
#include <limits>
#include <sstream>

Blob::Blob () {
  id = -1;
}

Blob::Blob (int _color, long _id) {
  color = _color;
  x_min = numeric_limits<int>::max();
  x_max = numeric_limits<int>::min();
  y_min = numeric_limits<int>::max();
  y_max = numeric_limits<int>::min();
  x_velocity = 0; y_velocity = 0;
  parent_id = -1; child_id = -1;
  id = _id;
};

void Blob::update_minmax(int x, int y) {
  x_min = min(x, x_min);
  x_max = max(x, x_max);
  y_min = min(y, y_min);
  y_max = max(y, y_max);
};

void Blob::add_pixel(int x, int y) {
  point p(x,y);
  mask.insert(p);
  update_minmax(x,y);
};

void Blob::add_neighbor(long neighbor_id) {
  neighbors.insert(neighbor_id);
};

point Blob::get_centroid() {
  return point((x_min+x_max)/2,(y_min+y_max)/2);
};

void Blob::consume(const Blob& other) {
  mask.insert(other.mask.begin(), other.mask.end());
  neighbors.insert(other.neighbors.begin(), other.neighbors.end());
  update_minmax(other.x_min, other.y_min);
  update_minmax(other.x_max, other.y_max);
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

float Blob::get_percentage_absolute_overlap(const Blob& other) {
  int overlap = 0;
  // Use the blob with fewer points for more efficiency
  if (mask.size() < other.mask.size()) {
    for (set<point>::iterator it=mask.begin(); it!=mask.end(); ++it)
      if (other.mask.find(*it) != other.mask.end())
        overlap++;
  } else {
    for (set<point>::iterator it=other.mask.begin(); it!=other.mask.end(); ++it)
      if (mask.find(*it) != mask.end())
        overlap++;
  }
  return overlap / (float) (mask.size() + other.mask.size() - overlap);
};

float Blob::get_percentage_relative_overlap(const Blob& other) {
  int overlap = 0;
  point p(0,0);
  if (mask.size() < other.mask.size()) {
    for (set<point>::iterator it=mask.begin(); it!=mask.end(); ++it) {
      p = *it; // Scale the point into the coordinates of the other blob
      p.x = p.x - x_min + other.x_min; 
      p.y = p.y - y_min + other.y_min;
      if (other.mask.find(p) != other.mask.end())
        overlap++;
    }
  } else {
    for (set<point>::iterator it=other.mask.begin(); it!=other.mask.end(); ++it) {
      p = *it; // Scale the point into our coordinates
      p.x = p.x - other.x_min + x_min;
      p.y = p.y - other.y_min + y_min;
      if (mask.find(*it) != mask.end())
        overlap++;
    }
  }
  return overlap / (float) (mask.size() + other.mask.size() - overlap);    
};

float Blob::get_aggregate_blob_match(const Blob& other) {
  int color_diff  = color == other.color;
  float dist_diff = get_centroid_dist(other);
  float size_diff = other.mask.size() / mask.size();

  float normalized_dist_diff = dist_diff > 8 ? 0 : 1 - (dist_diff / 9.0); // Slope adjusted from 8 to 9
  float normalized_size_diff = max(0.0f, 1.0f - abs(1.0f - size_diff));
  float match = (color_diff + normalized_size_diff + normalized_dist_diff) / 3.0f;
  return match;
  // Shape diff can be expensive -- only compute if the other variables look good
  //float shape_diff   = get_percentage_relative_overlap(other);
};

long Blob::find_matching_blob(map<long,Blob>& blobs, set<long>& excluded) {
  long best_match_id = -1;
  float best_match_score = 0;
  for (map<long,Blob>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
    Blob& b = it->second;
    if (excluded.find(b.id) != excluded.end())
      continue;
      
    float match = get_aggregate_blob_match(b);
    if (match > best_match_score || best_match_id < 0) {
      best_match_id = b.id;
      best_match_score = match;
    }
  }
  if (best_match_score < .5) {
    return -1;
  }

  return best_match_id;
};

void Blob::check_bounding_box() {
  assert(x_min >= 0);
  assert(y_min >= 0);
  for (set<point>::iterator it=mask.begin(); it!=mask.end(); it++) {
    assert(it->x >= x_min && it->x <= x_max);
    assert(it->y >= y_min && it->y <= y_max);
  }
};

void Blob::to_string() {
  printf("Blob: %p BB: (%d,%d)->(%d,%d) Size: %d Col: %d\n",this,x_min,y_min,x_max,y_max,(int)mask.size(),color);
};

Prototype::Prototype (CompositeObject& obj, map<long,Blob>& blob_map) {
  x_min = numeric_limits<int>::max();
  x_max = numeric_limits<int>::min();
  y_min = numeric_limits<int>::max();
  y_max = numeric_limits<int>::min();

  obj_ids.insert(obj.id);

  // Set the Prototype's pixel mask to that of the composite obj it was constructed from
  for (set<long>::iterator it=obj.blob_ids.begin(); it!=obj.blob_ids.end(); ++it) {
    long b_id = *it;
    assert(blob_map.find(b_id) != blob_map.end());
    Blob& b = blob_map[b_id];
    for (set<point>::iterator it2=b.mask.begin(); it2!=b.mask.end(); ++it2) {
      point p = *it2;
      p.x -= obj.x_min;
      p.y -= obj.y_min;

      // Update the bounding box
      x_min = min(p.x, x_min);
      x_max = max(p.x, x_max);
      y_min = min(p.y, y_min);
      y_max = max(p.y, y_max);

      assert(p.x >= 0 && p.y >= 0);
      assert(p.x <= obj.x_max-obj.x_min && p.y <= obj.y_max-obj.y_min);
      mask.insert(p);
    }
  }
  assert(x_min == 0 && y_min == 0);
  assert(x_max == obj.x_max - obj.x_min);
  assert(y_max == obj.y_max - obj.y_min);  
};


float Prototype::get_pixel_match(CompositeObject& obj, map<long,Blob>& blob_map) {
  int overlap = 0, total = 0;
  for (set<long>::iterator it=obj.blob_ids.begin(); it!=obj.blob_ids.end(); ++it) {
    long b_id = *it;
    assert(blob_map.find(b_id) != blob_map.end());
    Blob& b = blob_map[b_id];
    for (set<point>::iterator pit=b.mask.begin(); pit!=b.mask.end(); ++pit) {
      point p = *pit;
      p.y -= obj.y_min;
      p.x -= obj.x_min;
      if (mask.find(p) != mask.end())
        overlap++;
      total++;
    }
  }
  return overlap / (float) (total + mask.size() - overlap);
};

CompositeObject::CompositeObject() {
  id = -1;
};

CompositeObject::CompositeObject(int x_vel, int y_vel, long _id) {
  x_min = numeric_limits<int>::max();
  x_max = numeric_limits<int>::min();
  y_min = numeric_limits<int>::max();
  y_max = numeric_limits<int>::min();
  x_velocity = x_vel; y_velocity = y_vel;
  id = _id;
};

CompositeObject::~CompositeObject() {};

void CompositeObject::update_minmax(const Blob& b) {
  x_min = min(b.x_min, x_min);
  x_max = max(b.x_max, x_max);
  y_min = min(b.y_min, y_min);
  y_max = max(b.y_max, y_max);
};

void CompositeObject::add_blob(const Blob& b) {
  assert(b.x_velocity == x_velocity);
  assert(b.y_velocity == y_velocity);

  blob_ids.insert(b.id);
  update_minmax(b);
};

void CompositeObject::compute_boundingbox(map<long,Blob>& blob_map) {
  x_min = numeric_limits<int>::max();
  x_max = numeric_limits<int>::min();
  y_min = numeric_limits<int>::max();
  y_max = numeric_limits<int>::min();

  for (set<long>::iterator it=blob_ids.begin(); it!=blob_ids.end(); ++it) {
    long blob_id = *it;
    assert(blob_map.find(blob_id) != blob_map.end());
    Blob& b = blob_map[blob_id];
    update_minmax(b);
  }
};

void CompositeObject::check_bounding_box(map<long,Blob>& blob_map) {
  for (set<long>::iterator it=blob_ids.begin(); it!=blob_ids.end(); ++it) {
    long b_id = *it;
    assert(blob_map.find(b_id) != blob_map.end());
    Blob& b = blob_map[b_id];
    for (set<point>::iterator it2=b.mask.begin(); it2!=b.mask.end(); ++it2) {
      assert(it2->y >= y_min && it2->y <= y_max);
      assert(it2->x >= x_min && it2->x <= x_max);
    }
  }
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

float CompositeObject::get_pixel_match(const CompositeObject& other) {
  // set<point> other_mask;
  // for (set<Blob*>::iterator it=other.blobs.begin(); it!=other.blobs.end(); ++it) {
  //   Blob* b = *it;
  //   for (set<point>::iterator pit=b->mask.begin(); pit!=b->mask.end(); ++pit) {
  //     point p = *pit;
  //     p.y -= other.y_min;
  //     p.x -= other.x_min;
  //     other_mask.insert(p);
  //   }
  // }
  // // Compare the two masks
  // int overlap, total = 0;
  // for (set<Blob*>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
  //   Blob* b = *it;
  //   for (set<point>::iterator pit=b->mask.begin(); pit!=b->mask.end(); ++pit) {
  //     point p = *pit;
  //     p.y -= y_min;
  //     p.x -= x_min;
  //     if (other_mask.find(p) != other_mask.end())
  //       overlap++;
  //     total++;
  //   }
  // }
  // return overlap / (float) (total + other_mask.size() - overlap);
  return 0;
};


SelfDetectionAgent::SelfDetectionAgent(GameSettings* _game_settings, OSystem* _osystem) : 
  PlayerAgent(_game_settings, _osystem),
  max_history_len(50), //numeric_limits<int>::max()),
  curr_num_regions(0), prev_num_regions(0), blob_ids(0), obj_ids(0), self_id(-1)
{

  // Get the height and width of the screen
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  screen_width  = mediasrc.width();
  screen_height = mediasrc.height();

  // initilize the region matrix
  for (int y = 0; y < screen_height; y++) {
    IntVect row;
    for (int x = 0; x < screen_width; x++) {
      row.push_back(-1);
    }
    region_matrix.push_back(row);
  }
  curr_num_regions = 0;
  prev_num_regions = 0;

  // Parameters used for shape tracking
  f_max_perc_difference = p_osystem->settings().getFloat("max_perc_difference", true);
  i_max_obj_velocity = p_osystem->settings().getInt("max_obj_velocity", true);
  f_max_shape_area_dif = p_osystem->settings().getFloat("max_shape_area_dif", true);
};

// Returns a random action from the set of possible actions
Action SelfDetectionAgent::agent_step(const IntMatrix* screen_matrix, 
                                      const IntVect* console_ram, 
                                      int frame_number) {
  Action action = PlayerAgent::agent_step(screen_matrix, console_ram, frame_number);
  if (action == UNDEFINED)
     action = choice <Action>(p_game_settings->pv_possible_actions);

  curr_blobs.clear();
  find_connected_components(*screen_matrix, curr_blobs);

  if (blob_hist.size() > 1) {
    find_blob_matches(curr_blobs);

    // Merge blobs into objects
    update_existing_objs();
    merge_blobs(curr_blobs);

    // Merge objects into classes
    merge_objects(.96);

    // Identify which object we are
    identify_self();
    assert(curr_blobs.find(self_id) != curr_blobs.end());
    Blob& self_blob = curr_blobs[self_id];

    // Graphical display stuff
    for (int y=0; y<screen_height; ++y) {
      for (int x=0; x<screen_width; ++x) {
        region_matrix[y][x] = 257;
      }
    }

    // Plot blobs
    // int region_color = 256;
    // for (map<long,Blob>::iterator it=curr_blobs.begin(); it!=curr_blobs.end(); ++it) {
    //   const Blob& b = it->second;
    //   region_color++;
    //   for (set<point>::iterator pit=b.mask.begin(); pit!=b.mask.end(); ++pit) {
    //     const point& p = *pit;
    //     region_matrix[p.y][p.x] = region_color;
    //   }
    // }

    // Plot objects
    // int region_color = 256;
    // for (int i=0; i<composite_objs.size(); ++i) {
    //   region_color++;
    //   CompositeObject& o = composite_objs[i];
    //   for (set<long>::iterator blob_it=o.blob_ids.begin(); blob_it!=o.blob_ids.end(); ++blob_it) {
    //     long b_id = *blob_it;
    //     assert(curr_blobs.find(b_id) != curr_blobs.end());
    //     Blob& b = curr_blobs[b_id];
    //     for (set<point>::iterator point_it=b.mask.begin(); point_it!=b.mask.end(); ++point_it) {
    //       point p = *point_it;
    //       region_matrix[p.y][p.x] = 0;//region_color;
    //     }
    //   }
    // }

    // Plot object classes
    int region_color = 256;
    for (int i=0; i<obj_classes.size(); ++i) {
      Prototype& p = obj_classes[i];
      region_color++;
      for (set<long>::iterator obj_it=p.obj_ids.begin(); obj_it!=p.obj_ids.end(); ++obj_it) {
        long o_id = *obj_it;
        assert(composite_objs.find(o_id) != composite_objs.end());
        CompositeObject& o = composite_objs[o_id];
        for (set<long>::iterator blob_it=o.blob_ids.begin(); blob_it!=o.blob_ids.end(); ++blob_it) {
          long b_id = *blob_it;
          assert(curr_blobs.find(b_id) != curr_blobs.end());
          Blob& b = curr_blobs[b_id];
          for (set<point>::iterator point_it=b.mask.begin(); point_it!=b.mask.end(); ++point_it) {
            point p = *point_it;
            region_matrix[p.y][p.x] = region_color;
          }
        }
      }
    }

    // Color the self blob
    for (set<point>::iterator it=self_blob.mask.begin(); it!=self_blob.mask.end(); ++it) {
      region_matrix[it->y][it->x] = 6;
    }
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

  return action;
}

void SelfDetectionAgent::process_image(const IntMatrix* screen_matrix, Action action) {
  curr_blobs.clear();
  find_connected_components(*screen_matrix, curr_blobs);

  if (blob_hist.size() > 1) {
    find_blob_matches(curr_blobs);

    // Merge blobs into objects
    update_existing_objs();
    merge_blobs(curr_blobs);

    // Merge objects into classes
    merge_objects(.96);

    // Identify which object we are
    identify_self();
    assert(curr_blobs.find(self_id) != curr_blobs.end());
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

void SelfDetectionAgent::find_connected_components(const IntMatrix& screen_matrix, map<long,Blob>& blob_map) {
  // initialize the blob matrix
  vector<vector<long> > blob_matrix; 
  for (int y = 0; y < screen_height; y++) {
    vector<long> row;
    for (int x = 0; x < screen_width; x++)
     row.push_back(-1);
    blob_matrix.push_back(row);
  }

  int num_neighbors = 4;
  int neighbors_y[] = {-1, -1, -1,  0};
  int neighbors_x[] = {-1,  0,  1, -1};
  // 1- First Scan
  int i, j, y, x, color_ind, neighbors_ind;
  set<long> matches;
  for (i=0; i<screen_height; ++i) {
    for (j=0; j<screen_width; ++j) {
      color_ind = screen_matrix[i][j];
      // find the region of i,j based on west and north neighbors.
      for (neighbors_ind = 0; neighbors_ind < num_neighbors; neighbors_ind++) {
        y = i + neighbors_y[neighbors_ind];
        x = j + neighbors_x[neighbors_ind];
        if (x < 0 || x >= screen_width || y < 0 || y >= screen_height)
          continue;
        if (screen_matrix[y][x] == color_ind) {
          matches.insert(blob_matrix[y][x]);
          assert(blob_matrix[y][x] >= 0);
        }
      }
      if (matches.empty()) {
        // this pixel is in a new region
        Blob b(color_ind, blob_ids++);
        b.add_pixel(j, i);
        blob_matrix[i][j] = b.id;
        blob_map.insert(pair<long,Blob>(b.id, b));
      } else if (matches.size() == 1) {
        long match_id = *matches.begin();
        blob_matrix[i][j] = match_id;
        blob_map[match_id].add_pixel(j, i);
      } else {
        // Multiple matches -- merge them
        int largest_blob_id = -1;
        for (set<long>::iterator it=matches.begin(); it!=matches.end(); ++it) {
          long blob_id = *it;
          if (largest_blob_id == -1 || blob_map[blob_id].mask.size() > blob_map[largest_blob_id].mask.size()) {
            largest_blob_id = blob_id;
          }
        }
        Blob& largest = blob_map[largest_blob_id];
        for (set<long>::iterator it=matches.begin(); it!=matches.end(); ++it) {
          long blob_id = *it;
          if (largest.id == blob_id)
            continue;
          Blob& b = blob_map[blob_id];
          for (set<point>::iterator pit = b.mask.begin(); pit!=b.mask.end(); pit++) {
            const point& p = *pit; //b->mask[l];
            assert(blob_matrix[p.y][p.x] == b.id);
            blob_matrix[p.y][p.x] = largest.id;
          }
          largest.consume(b);
          blob_map.erase(b.id);
        }
        largest.add_pixel(j,i);
        blob_matrix[i][j] = largest.id;
      }
      matches.clear();
    }
  }

  // Populate neighbors
  for (int i = 0; i < screen_height; i++) {
    for (int j = 0; j < screen_width; j++) {
      int b_id = blob_matrix[i][j];
      Blob& b = blob_map[b_id];
      if (j+1 < screen_width && b_id != blob_matrix[i][j+1]) {
        b.add_neighbor(blob_matrix[i][j+1]);
        blob_map[blob_matrix[i][j+1]].add_neighbor(b.id);
      }
      if (i+1 < screen_height && b_id != blob_matrix[i+1][j]) {
        b.add_neighbor(blob_matrix[i+1][j]);
        blob_map[blob_matrix[i+1][j]].add_neighbor(b.id);
      }
    }
  }
};

void SelfDetectionAgent::find_blob_matches(map<long,Blob>& blobs) {
  // Try to match all of the current blobs with the equivalent blobs from the last timestep
  map<long,Blob>& old_blobs = blob_hist.back();
  set<long> excluded;
  for (map<long,Blob>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
    Blob& b = it->second;
    long blob_match_id = b.find_matching_blob(old_blobs,excluded);
    if (blob_match_id >= 0) { // Ad-hoc code to decide which match has priority
      assert(old_blobs.find(blob_match_id) != old_blobs.end());
      Blob& match = old_blobs[blob_match_id];

      if (match.child_id < 0) { // Easy Case
        b.compute_velocity(match);
        b.parent_id = match.id;
        match.child_id = b.id;
      } else { // Hard case
        long conflicting_blob_id = match.child_id;
        Blob& conflicting_blob = blobs[conflicting_blob_id];
        assert(conflicting_blob.parent_id == match.id);
        excluded.clear();
        excluded.insert(match.id);

        // Compute primary and secondary blob match scores for b
        float primary_score = b.get_aggregate_blob_match(match);
        long  second_choice_id = b.find_matching_blob(blob_hist.back(),excluded);
        float secondary_score = 0;
        if (second_choice_id >= 0) {
          assert(blob_hist.back().find(second_choice_id) != blob_hist.back().end());
          Blob& second_choice = blob_hist.back()[second_choice_id];
          secondary_score = b.get_aggregate_blob_match(second_choice);
        } 

        // Compute primary and secondary blob match scores for conflicting blob
        float primary_score2 = conflicting_blob.get_aggregate_blob_match(match);
        long  second_choice2_id = conflicting_blob.find_matching_blob(blob_hist.back(), excluded);
        float secondary_score2 = 0;
        if (second_choice2_id >= 0) {
          assert(blob_hist.back().find(second_choice2_id) != blob_hist.back().end());
          Blob& second_choice2 = blob_hist.back()[second_choice2_id];
          secondary_score2 = conflicting_blob.get_aggregate_blob_match(second_choice2);
        }

        if (primary_score + secondary_score2 >= primary_score2 + secondary_score) {
          // b takes the primary and conflicting takes the secondary
          b.compute_velocity(match);
          b.parent_id = match.id;
          match.child_id = b.id;
          if (second_choice2_id >= 0) {
            Blob& second_choice2 = blob_hist.back()[second_choice2_id];
            conflicting_blob.compute_velocity(second_choice2);
            conflicting_blob.parent_id = second_choice2.id;
            second_choice2.child_id = conflicting_blob.id;
          }
        } else {
          // b takes the secondary and conflicting takes the primary
          if (second_choice_id >= 0) {
            Blob& second_choice = blob_hist.back()[second_choice_id];
            b.compute_velocity(second_choice);
            b.parent_id = second_choice.id;
            second_choice.child_id = b.id;
          }
          conflicting_blob.compute_velocity(match);
          conflicting_blob.parent_id = match.id;
          match.child_id = conflicting_blob.id;
        }
      } 
    }
  }
};

// Merge equivalent blobs into a single composite object
void SelfDetectionAgent::merge_blobs(map<long,Blob>& blobs) {
  set<long> checked;
  
  // Add all the blobs in the existing objects to the list of checked blobs
  for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
    CompositeObject& obj = it->second;
    checked.insert(obj.blob_ids.begin(), obj.blob_ids.end());
  }

  for (map<long,Blob>::iterator it=blobs.begin(); it!=blobs.end(); ++it) {
    Blob& b = it->second;

    if (checked.find(b.id) != checked.end())
      continue;
    
    // Only create blobs when velocity is greater than zero
    if (b.x_velocity != 0 || b.y_velocity != 0) {
      CompositeObject obj(b.x_velocity, b.y_velocity, obj_ids++);
      obj.add_blob(b);
      obj.expand(curr_blobs);
      composite_objs[obj.id] = obj;
      checked.insert(obj.blob_ids.begin(), obj.blob_ids.end());
    }
    checked.insert(b.id);
  }
};

// Update the current blobs that we already have
void SelfDetectionAgent::update_existing_objs() {
  set<long> used_blob_ids; // A blob becomes used when it is integrated into an existing object
  set<long> new_blob_ids; // Blobs who have children in the current timestep
  vector<long> to_remove;
  map<long,Blob>& old_blobs = blob_hist.back();

  for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
    CompositeObject& obj = it->second;
    new_blob_ids.clear();
    
    // Update the blobs to their equivalents in the next frame
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
      continue;
    }

    // Checks that the object is still cohesive and velocity consistent
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
        break;
      }
    }

    // This works if object is still cohesive
    if (velocity_consistent) {
      obj.blob_ids.clear();
      obj.blob_ids.insert(new_blob_ids.begin(), new_blob_ids.end());
      obj.x_velocity = first.x_velocity;
      obj.y_velocity = first.y_velocity;
      // Expand the object if it is non-stationary
      if (obj.x_velocity != 0 || obj.y_velocity != 0)
        obj.expand(curr_blobs);
      obj.compute_boundingbox(curr_blobs); // Recompute bounding box
      used_blob_ids.insert(obj.blob_ids.begin(), obj.blob_ids.end());
    } else {
      // This object is no longer legitimate. Decide what to do with it.
      to_remove.push_back(obj.id);
      continue;
    }
  }

  // Walk through list in reverse removing objects
  for (int i=0; i<to_remove.size(); ++i) {
    assert(composite_objs.find(to_remove[i]) != composite_objs.end());
    composite_objs.erase(to_remove[i]); 
  }
};

void SelfDetectionAgent::identify_self() {
  float max_info_gain = -1;
  long best_blob_id = -1;
  for (map<long,Blob>::iterator it=curr_blobs.begin(); it!=curr_blobs.end(); ++it) {
    long b_id = it->first;

    int blob_history_len = 0;
    vector<pair<int,int> > velocity_hist;
    map<pair<int,int>,int> velocity_counts;

    assert(curr_blobs.find(b_id) != curr_blobs.end());
    Blob* b = &curr_blobs[b_id];

    // Get the velocity history of this blob
    while (b->parent_id >= 0 && blob_history_len < max_history_len) {
      // Push back the velocity
      pair<int,int> vel(b->x_velocity, b->y_velocity);
      velocity_hist.push_back(vel);
      velocity_counts[vel] = GetWithDef(velocity_counts,vel,0) + 1;

      blob_history_len++;

      // Get the parent
      map<long,Blob>& old_blobs = blob_hist[blob_hist.size() - blob_history_len];
      long parent_id = b->parent_id;
      assert(old_blobs.find(parent_id) != old_blobs.end());
      b = &old_blobs[parent_id];
    }

    // How many times was each action performed?
    map<Action,int> action_counts;
    vector<Action> act_vec;
    for (int i=0; i<blob_history_len; ++i) {
      Action a = action_hist[action_hist.size()-i-1];
      act_vec.push_back(a);
      action_counts[a] = GetWithDef(action_counts,a,0) + 1;
    }

    assert(act_vec.size() == velocity_hist.size());

    // Calculate H(velocities)
    float velocity_entropy = compute_entropy(velocity_counts,blob_history_len);

    // Calculate H(velocity|a)
    float action_entropy = 0;
    for (map<Action,int>::iterator it2=action_counts.begin(); it2!=action_counts.end(); ++it2) {
      Action a = it2->first;
      int count = it2->second;
      float p_a = count / (float) blob_history_len;
      map<pair<int,int>,int> selective_counts;
      int selective_total = 0;
      for (int i=0; i<blob_history_len; ++i) {
        if (act_vec[i] == a) {
          pair<int,int> vel = velocity_hist[i];
          selective_counts[vel] = GetWithDef(selective_counts,vel,0) + 1;
          selective_total++;
        }
      }
      float selective_entropy = compute_entropy(selective_counts,selective_total);
      action_entropy += p_a * selective_entropy;
    }

    float info_gain = velocity_entropy - action_entropy;
    if (info_gain > max_info_gain) {
      max_info_gain = info_gain;
      best_blob_id = b_id;
    }
  }
  //printf("Max info gain: %f\n",max_info_gain);
  //best_blob->to_string();
  self_id = best_blob_id;
};

point SelfDetectionAgent::get_self_centroid() {
  if (curr_blobs.find(self_id) == curr_blobs.end())
    return point(-1,-1);
  return curr_blobs[self_id].get_centroid();
};


// Merges together objects into classes of objects
void SelfDetectionAgent::merge_objects(float similarity_threshold) {
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

      CompositeObject& obj = composite_objs[obj_id];
      float pixel_match = p.get_pixel_match(obj, curr_blobs);
      if (pixel_match >= similarity_threshold) {
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

  for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); it++) {
    if (checked_objs.find(it->first) != checked_objs.end())
      continue; 
    CompositeObject& obj = it->second;
    // See if any of the existing prototypes match this object
    bool found_match = false;
    for (int j=0; j<obj_classes.size(); ++j) {
      Prototype& p = obj_classes[j];
      float pixel_match = p.get_pixel_match(obj, curr_blobs);
      if (pixel_match > .99) {
        p.obj_ids.insert(obj.id);
        p.times_seen_this_frame++; //(piyushk)
        found_match = true;
        break;
      }
    }
    
    if (!found_match) {
      Prototype p(obj,curr_blobs);
      p.seen_count = p.frames_since_last_seen = 0; //(piyushk)
      obj_classes.push_back(p);
    }
  }

  //(piyushk) remove bad prototypes here
  vector<int> prototypes_to_erase;
  for (int i=0; i<obj_classes.size(); ++i) {
    Prototype& p = obj_classes[i];
    if (!p.times_seen_this_frame)
      p.frames_since_last_seen++;
    else
      p.frames_since_last_seen = 0;
    if (p.seen_count < 5 && p.frames_since_last_seen > 3) {
      prototypes_to_erase.push_back(i);
    }
  }
  for (int i = prototypes_to_erase.size() - 1; i >= 0; --i) {
    obj_classes.erase(obj_classes.begin() + i);
  }

  std::cout << "Active Prototypes: " << obj_classes.size() << std::endl;
  
};

// Overrides the normal display screen method to alter our display
void SelfDetectionAgent::display_screen(const IntMatrix& screen_matrix) {
  IntMatrix screen_cpy(screen_matrix);
  //cin.get();
  plot_regions(screen_cpy);
  PlayerAgent::display_screen(screen_cpy);
};

int SelfDetectionAgent::get_num_regions(IntMatrix& regions) {
  map<int,bool> found;
  for (int y=0; y<screen_height; ++y)
    for (int x=0; x<screen_width; ++x)
      found[regions[y][x]] = true;
  return found.size();
};

void SelfDetectionAgent::plot_regions(IntMatrix& screen_matrix) {
  map<int,int> col_map;
  for (int y=0; y<screen_height; ++y) {
    for (int x=0; x<screen_width; ++x) {
      int reg_num = region_matrix[y][x];
      screen_matrix[y][x] = reg_num;
    }
  }
};
