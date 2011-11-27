/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************/

#ifndef NODE_H
#define NODE_H

#include "common_constants.h"

//typedef vector<Node*> TreeNodeList;

class Node {
  /* *************************************************************************
     Represents a node in the uct search tree
     ************************************************************************* */
 public:
  /* *********************************************************************
     Constructor
     Generates a new tree node by starting from start_state and 
     simulating the game for num_simulate_steps steps.
     ******************************************************************* */
  Node(Node* parent);

  /* *********************************************************************
     Returns true if this is a leaf node
     ******************************************************************* */
  bool is_leaf(void) { return (v_children.empty()); };
		
  Node* p_parent;    // pointer to our parent
  float f_branch_reward; // best reward possible in this branch = node_reward + max(children.branch_reward) or node_reward + avg(children.branch_reward) 	
  int i_best_branch;	// Best sub-branch that can be taken from the current node
  float f_uct_value;	// This is the UCT value, which helps us decide to eitehr explore or exploit
  int i_uct_visit_count;	// How many times we have visited this node  
  vector<Node*> v_children;
  //  TreeNodeList v_children;  // vector of children nodes
  float f_uct_sum_reward;	// Sum of the rewards we have recieved through all simulations from this node
  int depth;  // Depth in the tree. 
};

#endif
