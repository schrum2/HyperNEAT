/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  tree_node.cpp
 *
 *  Implementation of the TreeNode class, which represents a node in the search-
 *	tree for the Search-Agent
 **************************************************************************** */

#include "tree_node.h"
#include "search_agent.h"


/* *********************************************************************
	Constructor
	Generates a new tree node by starting from start_state and 
	simulating the game for num_simulate_steps steps.
 ******************************************************************* */
TreeNode::TreeNode(	TreeNode* parent, const string start_state, 
					int start_frame_num, 
					SearchAgent* search_agent, 
					int num_simulate_steps, Action a):
  p_parent(parent),
  str_state(""),
  i_frame_num(-1),
  f_node_reward(0.0), 
  f_branch_reward(0.0),
  i_best_branch(-1), 
  b_is_dead(false),
  f_uct_value(0.0),
  i_uct_visit_count(0),
  i_uct_death_count(0),
  f_uct_sum_reward(0.0)  {
  // Simulate the game for si_num_sim_steps
  search_agent->load_state(start_state);
  search_agent->simulate_game(a, num_simulate_steps, start_frame_num, 
                              f_node_reward, b_is_dead);
  str_state = search_agent->save_state();
  i_frame_num = start_frame_num + num_simulate_steps;
  f_branch_reward = f_node_reward;	// we don't have any children yet
}

