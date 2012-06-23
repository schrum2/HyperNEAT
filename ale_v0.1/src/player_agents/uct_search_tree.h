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
 *  Implementation of the UCTSearchTree class, an implementation of the 
 *  Upper Confidence Bound for Trees algorithm for the Rollout agent
 **************************************************************************** */

#ifndef UCT_SEARCH_TREE_H
#define UCT_SEARCH_TREE_H

#include "search_tree.h"



class UCTSearchTree : public SearchTree {
  /* *************************************************************************
     Represents search a Full-Search-Tree (i.e. a search-tree taht expands 
     all its nodes until the give ndebth), used by the Search Agent
     ************************************************************************* */
	
 public:
  /* *********************************************************************
     Constructor
     ******************************************************************* */
  UCTSearchTree(SearchAgent* search_agent);

  /* *********************************************************************
     Deconstructor
     ******************************************************************* */
  virtual ~UCTSearchTree();

  /* *********************************************************************
     Builds a new tree
     ******************************************************************* */
  virtual void build(	const string start_state, int start_frame_num);

  /* *********************************************************************
     Re-Expands the tree until i_max_sim_steps_per_tree is reached
     ******************************************************************* */
  virtual void update_tree(void);

  /* *********************************************************************
     Returns the best action based on the expanded search tree
     ******************************************************************* */
  virtual Action get_best_action(void);	

  /* *********************************************************************
     Prints the Search-Tree, starting from the given node
     if node is NULL (default), we will start from the root
     ******************************************************************* */
  virtual void print(TreeNode* node = NULL) const;
				
 protected:	

  /* *********************************************************************
     Performs a single UCT iteration, starting from the root
     ******************************************************************* */
  virtual void single_uct_iteration(void);

  /* *********************************************************************
     Returns the index of the first child with zero count
     Returns -1 if no such child is found
     ******************************************************************* */
  int get_child_with_count_zero(const TreeNode* node) const;
		
  /* *********************************************************************
     Returns the sub-branch with the highest value
     if add_exp_explt_val is truem we will add the UCT's
     Exploration-Exploitation to each branch value and then take the max
     ******************************************************************* */
  int get_best_branch(TreeNode* node, bool add_exp_explt_val);
		
  /* *********************************************************************
     Expands the given node, by generating all its children
     ******************************************************************* */
  void expand_node(TreeNode* node);

  /* *********************************************************************
     Performs a Monte Carlo simulation from the given node, for
     i_uct_monte_carlo_steps steps 
     ******************************************************************* */
  void do_monte_carlo(TreeNode* start_node, float& reward, bool& is_dead);

  /* *********************************************************************
     Update the node values and counters from the current node, all the
     way up to the root
     ******************************************************************* */
  void update_values(TreeNode* node, float reward, bool is_dead);
		

  int i_uct_monte_carlo_steps;// Number of simulated Monte Carlo steps that will be run on each UCT iteration
  float f_uct_exploration_const;	// Exploration Constant
  int i_uct_min_death_count;	// Minimum number of simulations that should end up dead from a node, before we mark that node as dead.
  bool b_branch_value_average;// When true, we will take the abverage value 
  // of a node's children as its branch value
  // when false, we will take the max
  bool b_avg_reward_per_frame;// When true, uct will look at 
  // reward/frame (not just reward). 
  // This is to prevent"	biasing towards 
  // exploring already deeper sub-branches
};



#endif
