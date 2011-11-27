/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  uct_search_tree.h
 *
 *  Implementation of the MdlUCTSearchTree class, an implementation of the 
 *  Upper Confidence Bound for Trees algorithm for the Rollout agent
 **************************************************************************** */

#ifndef UCT_H
#define UCT_H

#include "../models/model.h"
#include "../player_agents/common_constants.h"
#include "Settings.hxx"
#include "node.h"

class UCT {
 public:
  UCT(Settings& settings, ActionVect* possible_actions);//ModelAgent* model_agent);
  virtual ~UCT();

  // Builds a new tree
  virtual void build(const Model& model, const IntArr& current_state);

  // Deletes the search-tree
  void clear(void);

  // Moves the best sub-branch of the root to be the new root of the tree
  void move_to_best_sub_branch(void);

  // Re-Expands the tree until i_max_sim_steps_per_tree is reached
  virtual void update_tree(const Model& model, const IntArr& current_state);

  // Takes a single step with the model
  void sim_step(Model* curr_model, int actionIndx, float& reward, IntArr& curr_state, IntArr& next_state);
  // Same as above, just with the actual action
  void sim_step(Model* curr_model, Action a, float& reward, IntArr& curr_state, IntArr& next_state);

  // Returns the best action based on the expanded search tree
  virtual Action get_best_action(void);	

  //   Prints the Search-Tree, starting from the given node if node is NULL (default), we will start from the root
  virtual void print(Node* node = NULL) const;
				
  // Deletes a node and all its children, all the way down the branch
  void delete_branch(Node* node);

  bool is_built;		// True whe the tree is built
  int i_deepest_node_frame_num; // the frame number for the deepest node

 protected:	

  // Performs a single UCT iteration, starting from the root
  virtual void single_uct_iteration(const Model& model, const IntArr& start_state);

  // Returns the index of the first child with zero count. Returns -1 if no such child is found
  int get_child_with_count_zero(const Node* node) const;
		
  //   Returns the sub-branch with the highest value
  //   if add_exp_explt_val is truem we will add the UCT's
  //   Exploration-Exploitation to each branch value and then take the max
  int get_best_branch(Node* node, bool add_exp_explt_val);
		
  // Expands the given node, by generating all its children
  void expand_node(Node* node);

  // Performs a Monte Carlo simulation from the given node, for i_uct_monte_carlo_steps steps 
  void do_monte_carlo(Node* start_node, float& reward, Model* model, IntArr& curr_state, IntArr& next_state);

  // Update the node values and counters from the current node, all the way up to the root
  void update_values(Node* node, float reward);
		

  //ModelAgent* p_model_agent;	// Pointer to the search-agent
  ActionVect* possible_actions; // Vector of possible actions for this game
  Node* p_root;		// Root of the SearchTree
  int curr_sim_steps; // Count of the number of sim steps we've taken so far this iteration.
  int num_steps_between_nodes;	// Number of steps we will run the simulation in each search-tree node
  int max_sim_steps; //Maximum total number of simulation steps that will be carried to per iteration
  float f_discount_factor;// Discount factor to force the tree prefer closer goals
  int max_monte_carlo_steps;// Number of simulated Monte Carlo steps that will be run on each playout
  float f_uct_exploration_const;	// Exploration Constant
  bool b_branch_value_average;// When true, we will take the abverage value 
  // of a node's children as its branch value
  // when false, we will take the max
  bool b_avg_reward_per_frame;// When true, uct will look at 
  // reward/frame (not just reward). 
  // This is to prevent"	biasing towards 
  // exploring already deeper sub-branches

  long numNodes;
  int numDeletedNodes;
  int numAddedNodes; 
};



#endif
