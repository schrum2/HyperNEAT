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
#include <omp.h>

SelfDetectionAgent::SelfDetectionAgent(GameSettings* _game_settings, OSystem* _osystem) : 
  PlayerAgent(_game_settings, _osystem), visProc(_osystem)
{
  // Get the height and width of the screen
  MediaSource& mediasrc = p_osystem->console().mediaSource();
  screen_width  = mediasrc.width();
  screen_height = mediasrc.height();
};

// Returns a random action from the set of possible actions
Action SelfDetectionAgent::agent_step(const IntMatrix* screen_matrix, 
                                      const IntVect* console_ram, 
                                      int frame_number) {
  Action action = PlayerAgent::agent_step(screen_matrix, console_ram, frame_number);
  if (action == UNDEFINED)
     action = choice <Action>(p_game_settings->pv_possible_actions);

  visProc.process_image(screen_matrix,action);

  return action;
}

// void SelfDetectionAgent::identify_self() {
//   float max_info_gain = -1;
//   long best_obj_id = -1;
//   int oldest = 0;
//   for (map<long,CompositeObject>::iterator it=composite_objs.begin(); it!=composite_objs.end(); ++it) {
//     long obj_id = it->first;
//     CompositeObject& obj = it->second;
//     //cout <<"Obj: " << obj_id << " age " << obj.age<< endl;
//     for (set<long>::iterator bit=obj.blob_ids.begin(); bit!=obj.blob_ids.end(); ++bit) {
//       long b_id = *bit;
//       assert(curr_blobs.find(b_id) != curr_blobs.end());
//       int hist_len = 0;
//       Blob* b = &curr_blobs[b_id];
//       //cout << "\tBlob " << b_id << " (" << b->x_velocity << "," << b->y_velocity << ") ";
//       while (b->parent_id >= 0 && hist_len < max_history_len) {
//         hist_len++;
//         map<long,Blob>& old_blobs = blob_hist[blob_hist.size() - hist_len];
//         long parent_id = b->parent_id;
//         assert(old_blobs.find(parent_id) != old_blobs.end());
//         b = &old_blobs[parent_id];
//         //cout << " (" << b->x_velocity << "," << b->y_velocity << ") ";
//       }
//       //cout << endl;
//       //cout << "\tHistoryLen: " << hist_len << endl;
//     }
//   }
//   //cin.get();
// };

void SelfDetectionAgent::identify_self() {
  // float max_info_gain = -1;
  // long best_blob_id = -1;
  // for (map<long,Blob>::iterator it=curr_blobs.begin(); it!=curr_blobs.end(); ++it) {
  //   long b_id = it->first;

  //   int blob_history_len = 0;
  //   vector<pair<int,int> > velocity_hist;
  //   map<pair<int,int>,int> velocity_counts;

  //   assert(curr_blobs.find(b_id) != curr_blobs.end());
  //   Blob* b = &curr_blobs[b_id];

  //   // Get the velocity history of this blob
  //   while (b->parent_id >= 0 && blob_history_len < max_history_len) {
  //     // Push back the velocity
  //     pair<int,int> vel(b->x_velocity, b->y_velocity);
  //     velocity_hist.push_back(vel);
  //     velocity_counts[vel] = GetWithDef(velocity_counts,vel,0) + 1;

  //     blob_history_len++;

  //     // Get the parent
  //     map<long,Blob>& old_blobs = blob_hist[blob_hist.size() - blob_history_len];
  //     long parent_id = b->parent_id;
  //     assert(old_blobs.find(parent_id) != old_blobs.end());
  //     b = &old_blobs[parent_id];
  //   }

  //   // How many times was each action performed?
  //   map<Action,int> action_counts;
  //   vector<Action> act_vec;
  //   for (int i=0; i<blob_history_len; ++i) {
  //     Action a = action_hist[action_hist.size()-i-1];
  //     act_vec.push_back(a);
  //     action_counts[a] = GetWithDef(action_counts,a,0) + 1;
  //   }

  //   assert(act_vec.size() == velocity_hist.size());

  //   // Calculate H(velocities)
  //   float velocity_entropy = compute_entropy(velocity_counts,blob_history_len);

  //   // Calculate H(velocity|a)
  //   float action_entropy = 0;
  //   for (map<Action,int>::iterator it2=action_counts.begin(); it2!=action_counts.end(); ++it2) {
  //     Action a = it2->first;
  //     int count = it2->second;
  //     float p_a = count / (float) blob_history_len;
  //     map<pair<int,int>,int> selective_counts;
  //     int selective_total = 0;
  //     for (int i=0; i<blob_history_len; ++i) {
  //       if (act_vec[i] == a) {
  //         pair<int,int> vel = velocity_hist[i];
  //         selective_counts[vel] = GetWithDef(selective_counts,vel,0) + 1;
  //         selective_total++;
  //       }
  //     }
  //     float selective_entropy = compute_entropy(selective_counts,selective_total);
  //     action_entropy += p_a * selective_entropy;
  //   }

  //   float info_gain = velocity_entropy - action_entropy;
  //   if (info_gain > max_info_gain) {
  //     max_info_gain = info_gain;
  //     best_blob_id = b_id;
  //   }
  // }
  // //printf("Max info gain: %f\n",max_info_gain);
  // //best_blob->to_string();
  // self_id = best_blob_id;
};

void SelfDetectionAgent::printVelHistory(CompositeObject& obj) {
  // for (set<long>::iterator it=obj.blob_ids.begin(); it!=obj.blob_ids.end(); ++it) {
  //   long b_id = *it;
    
  //   assert(curr_blobs.find(b_id) != curr_blobs.end());
  //   Blob* b = &curr_blobs[b_id];
  //   printf("Blob %ld: ",b_id);
  //   // Get the velocity history of this blob
  //   int blob_history_len = 1;
  //   while (b->parent_id >= 0 && blob_history_len < max_history_len) {
  //     // Push back the velocity
  //     pair<int,int> vel(b->x_velocity, b->y_velocity);
  //     blob_history_len++;
  //     string action_name = action_to_string(action_hist[action_hist.size() - blob_history_len]);
  //     printf("%s (%d,%d)\n",action_name.c_str(), b->x_velocity, b->y_velocity);
  //     // Get the parent
  //     map<long,Blob>& old_blobs = blob_hist[blob_hist.size() - blob_history_len];
  //     long parent_id = b->parent_id;
  //     assert(old_blobs.find(parent_id) != old_blobs.end());
  //     b = &old_blobs[parent_id];
  //   }
  //   printf("\n");
  // }  
};


// Overrides the normal display screen method to alter our display
void SelfDetectionAgent::display_screen(const IntMatrix& screen_matrix) {
  IntMatrix screen_cpy(screen_matrix);
  visProc.display_screen(screen_cpy);
  PlayerAgent::display_screen(screen_cpy);
};

