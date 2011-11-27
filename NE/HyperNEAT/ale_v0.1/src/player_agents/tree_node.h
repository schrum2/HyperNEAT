/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  tree_node.h
 *
 *  Implementation of the TreeNode class, which represents a node in the search-
 *	tree for the Search-Agent
 **************************************************************************** */

#ifndef TREE_NODE_H
#define TREE_NODE_H

#include "common_constants.h"
class SearchAgent;
class TreeNode;
typedef vector<TreeNode*> NodeList;

class TreeNode {
  /* *************************************************************************
     Represents a node in the search-tree for the Search-Agent
     ************************************************************************* */

 public:
  /* *********************************************************************
     Constructor
     Generates a new tree node by starting from start_state and 
     simulating the game for num_simulate_steps steps.
     ******************************************************************* */
  TreeNode(	TreeNode* parent,  const string start_state, 
                int start_frame_num, SearchAgent* search_agent, 
                int num_simulate_steps, Action a);	

  /* *********************************************************************
     Returns true if this is a leaf node
     ******************************************************************* */
  bool is_leaf(void) {
    return (v_children.empty());
  }
		
  TreeNode* p_parent;	// pointer to our parent
  string str_state;		// The state of current node
  int i_frame_num;	// The frame number for the state of this node. 
  float f_node_reward;	// reward recieved in this node
  float f_branch_reward;	// best reward possible in this branch
  // = node_reward + max(children.branch_reward)
  // or node_reward + avg(children.branch_reward) 	
  int i_best_branch;	// Best sub-branch that can be taken 
  // from the current node
  bool b_is_dead;		// true when either the game ended in
  // this node, or all children are dead
  NodeList v_children;// vector of children nodes
  float f_uct_value;	// This is the UCT value, which helps us decide
  // to eitehr explore or exploit
  int i_uct_visit_count;	// How many times we have visited this node
  int i_uct_death_count;	// Out of the simulations we have run from this
  // node, how many times did we end up dead?
  float f_uct_sum_reward;	// Sum of the rewards we have recieved through
  //  all simulations from this node
};

#endif
