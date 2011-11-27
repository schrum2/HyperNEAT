/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  full_search_tree.cpp
 *
 *  Implementation of the FullSearchTree class, which represents search a
 *  Full-Search-Tree (i.e. a search-tree taht expands all its nodes until
 *  the give ndebth), used by the Search Agent
 * TODO: This code needs to be reviewed, as it is behaving strangely.
 **************************************************************************** */

#include "full_search_tree.h"
#include "search_agent.h"
#include <queue>

/* *********************************************************************
	Constructor
	Generates a whole search tree from the current state, until 
	max_frame_num is reached.
 ******************************************************************* */
FullSearchTree::FullSearchTree(SearchAgent* search_agent) :
	SearchTree(search_agent) {
	
}

/* *********************************************************************
	Deconstructor
 ******************************************************************* */
FullSearchTree::~FullSearchTree() {

}

/* *********************************************************************
	Builds a new tree
 ******************************************************************* */
void FullSearchTree::build(	const string start_state, 
							int start_frame_num) {
	assert(p_root == NULL);
	p_root = new TreeNode(NULL, start_state, start_frame_num, 
						  p_search_agent, 0, UNDEFINED);
	update_tree();
	is_built = true;				
}

/* *********************************************************************
	Re-Expands the tree until the new max_frame_num is reached on all 
	leaf nodes
 ******************************************************************* */
void FullSearchTree::update_tree() {
	expand_tree(p_root);
}

/* *********************************************************************
	Expands the tree from the given node until i_max_sim_steps_per_tree 
	is reached
	
	TODO: this is depth first. Change it back to breadth first!
 ******************************************************************* */
void FullSearchTree::expand_tree(TreeNode* start_node) { 

	queue<TreeNode*> q;
	q.push(start_node);
	while(!q.empty()) {
		if (p_search_agent->get_num_simulated_steps() > i_max_sim_steps_per_tree) {
			break;
		}
		TreeNode* curr_node = q.front();
		q.pop();
		for (int a = 0; a < p_search_agent->i_num_actions; a++) {
			Action act = (*p_search_agent->p_game_settings->pv_possible_actions)[a];
			TreeNode* new_child = new TreeNode(	curr_node,
												curr_node->str_state, 
												curr_node->i_frame_num, 
												p_search_agent, 
												i_sim_steps_per_node, 
												act);
			curr_node->v_children.push_back(new_child);
			if (new_child->i_frame_num > i_deepest_node_frame_num) {
				i_deepest_node_frame_num = new_child->i_frame_num;
			}
			if (!new_child->b_is_dead) {
				q.push(new_child);
			}
		}
	}
/*	
	start_node->b_is_dead = true;
	if (!start_node->v_children.empty()) {
		// This is not a leaf node
		for (unsigned int c = 0; c < start_node->v_children.size(); c++) {
			TreeNode* curr_child = start_node->v_children[c];
			expand_tree(curr_child);
			start_node->b_is_dead = ( start_node->b_is_dead && 
									  curr_child->b_is_dead );
		}
	} else {
		// This is a leaf node. Expand it.
		for (int a = 0; a < p_search_agent->i_num_actions; a++) {
			Action act = (*p_search_agent->p_game_settings->pv_possible_actions)[a];
			TreeNode* new_child = new TreeNode(	start_node,
												start_node->str_state, 
												start_node->i_frame_num, 
												p_search_agent, 
												i_sim_steps_per_node, 
												act);
			start_node->v_children.push_back(new_child);
			if (new_child->i_frame_num > i_deepest_node_frame_num) {
				i_deepest_node_frame_num = new_child->i_frame_num;
			}
			if (!new_child->b_is_dead) {
				expand_tree(new_child);
			}
			start_node->b_is_dead = ( start_node->b_is_dead && 
									  new_child->b_is_dead );
		}
	}
*/
	update_branch_reward(start_node);
}



/* *********************************************************************
	Updates the branch reward for the given node
	which equals to: node_reward + max(children.branch_reward)
 ******************************************************************* */
void FullSearchTree::update_branch_reward(TreeNode* node) {
	assert (!node->v_children.empty());

	// First, we have to make sure that all the children are updated
	node->b_is_dead = true;
	for (unsigned int c = 0; c < node->v_children.size(); c++) {
		TreeNode* curr_child = node->v_children[c];
		if (!curr_child->v_children.empty()) {
			update_branch_reward(curr_child);
		}
		node->b_is_dead = ( node->b_is_dead && curr_child->b_is_dead );
	}
	
	// Now that all the children are updated, we can update the branch-reward

	float best_branch_rew = -1;
	int best_branch = -1;
	if (node->b_is_dead) {
		// When all children are dead, we just pick the highest branch_reward
		for (unsigned int c = 0; c < node->v_children.size(); c++) {
			TreeNode* curr_child = node->v_children[c];
			if (best_branch == -1 || 
				curr_child->f_branch_reward > best_branch_rew) {
				best_branch_rew = curr_child->f_branch_reward;
				best_branch = c;
			}
		}
	} else {		
		// When at least one child is alive, we find the highest branch-reward
		// among the non-dead children
		unsigned  first_ind = -1;	// index of the first alive child
		for (first_ind = 0; first_ind < node->v_children.size(); first_ind++) {
			 if (!node->v_children[first_ind]->b_is_dead) {
				best_branch_rew = node->v_children[first_ind]->f_branch_reward;
				best_branch = first_ind;
				break;
			}
		}
		assert (first_ind != -1);
		for (unsigned int c = first_ind + 1; 
			 c < node->v_children.size(); c++) {
			if ((!node->v_children[c]->b_is_dead) &&  
				node->v_children[c]->f_branch_reward > best_branch_rew) {
				best_branch_rew = node->v_children[c]->f_branch_reward;
				best_branch = c;
			}
		}
	}
	node->f_branch_reward = ( node->f_node_reward + 
							  best_branch_rew * f_discount_factor);
	node->i_best_branch = best_branch;
	assert (node->b_is_dead == node->v_children[best_branch]->b_is_dead);
}


