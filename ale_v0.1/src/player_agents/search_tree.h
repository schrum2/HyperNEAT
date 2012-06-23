/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  search_tree.h
 *
 *  Implementation of the SearchTree class, which represents search tree for the
 *  Search-Agent
 **************************************************************************** */

#ifndef SEARCH_TREE_H
#define SEARCH_TREE_H

#include <queue>
#include "common_constants.h"
#include "tree_node.h"

class SearchAgent;


class SearchTree {
  /* *************************************************************************
     Represents a search tree for theRollout-Agent
     ************************************************************************* */
	
 public:
  /* *********************************************************************
     Constructor
     Generates a whole search tree from the current state, until 
     max_frame_num is reached.
     ******************************************************************* */
  SearchTree(SearchAgent* search_agent);

  /* *********************************************************************
     Deconstructor
     ******************************************************************* */
  virtual ~SearchTree();

  /* *********************************************************************
     Builds a new tree
     ******************************************************************* */
  virtual void build(	const string start_state, int start_frame_num) = 0;

  /* *********************************************************************
     Updates (re-expands) the tree. 
     ******************************************************************* */
  virtual void update_tree() = 0;
		
  /* *********************************************************************
     Deletes the search-tree
     ******************************************************************* */
  virtual void clear();
		
  /* *********************************************************************
     Returns the best action based on the expanded search tree
     ******************************************************************* */
  virtual Action get_best_action(void);	

  /* *********************************************************************
     Moves the best sub-branch of the root to be the new root of the tree
     ******************************************************************* */
  virtual void move_to_best_sub_branch(void);

		
  /* *********************************************************************
     Returns the frame number of the root
     ******************************************************************* */
  int get_root_frame_number(void) {
    return p_root->i_frame_num;
  }

  /* *********************************************************************
     Returns the the best branch-value for root
     ******************************************************************* */
  float get_root_value(void) {
    return p_root->v_children[p_root->i_best_branch]->f_branch_reward;
  }

  /* *********************************************************************
     Prints the Search-Tree, starting from the given node
     if node is NULL (default), we will start from the root
     ******************************************************************* */
  virtual void print(TreeNode* node = NULL) const;


  bool is_built;			// True whe the tree is built
  int i_deepest_node_frame_num; // the frame number for the deepest node
		
 protected:	


  /* *********************************************************************
     Deletes a node and all its children, all the way down the branch
     ******************************************************************* */
  void delete_branch(TreeNode* node);


  TreeNode* p_root;		// Root of the SearchTree
  int i_sim_steps_per_node;	// Number of steps we will run the 
  // simulation in each search-tree node
  int i_max_sim_steps_per_tree; //Maximum number of simulation steps that 
  // will be carried to build the tree
  float f_discount_factor;// Discount factor to force the tree prefer
  // closer goals
  SearchAgent* p_search_agent;	// Pointer to the search-agent
};



#endif
