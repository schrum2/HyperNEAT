/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  search_tree.cpp
 *
 *  Implementation of the SearchTree class, which represents search tree for the
 *  Search-Agent
 **************************************************************************** */

#include "search_tree.h"
#include "../common/random_tools.h"
#include "search_agent.h"


/* *********************************************************************
	Constructor
	Generates a whole search tree from the current state, until 
	max_frame_num is reached.
 ******************************************************************* */
SearchTree::SearchTree(	SearchAgent* search_agent): 
	is_built(false),
	p_root(NULL),
	p_search_agent(search_agent) {
	Settings& settings = p_search_agent->p_osystem->settings();
	i_sim_steps_per_node = settings.getInt("sim_steps_per_node", true);
	int max_sim_steps_per_frame = settings.getInt("max_sim_steps_per_frame",true);
	i_max_sim_steps_per_tree = max_sim_steps_per_frame * i_sim_steps_per_node;
	f_discount_factor = settings.getFloat("discount_factor", true);
	i_deepest_node_frame_num = 0;
}



/* *********************************************************************
	Deletes the search-tree
 ******************************************************************* */
void SearchTree::clear(void) {
	if (p_root != NULL) {
		delete_branch(p_root);
		p_root = NULL;
	}
	is_built = false;
	i_deepest_node_frame_num = 0;
}

/* *********************************************************************
	Deconstructor
 ******************************************************************* */
SearchTree::~SearchTree(void) {
	clear();
}

/* *********************************************************************
	Returns the best action based on the expanded search tree
 ******************************************************************* */
Action SearchTree::get_best_action(void) {
	assert (p_root != NULL);
	int best_branch = p_root->i_best_branch;
	TreeNode* best_child = p_root->v_children[best_branch];
	assert(best_branch != -1);
	vector<int> best_branches;
	best_branches.push_back(best_branch);
	for (unsigned int c = 0; c < p_root->v_children.size(); c++) {
		TreeNode* curr_child = p_root->v_children[c];
		if (c != best_branch && 
			curr_child->f_branch_reward == best_child->f_branch_reward  && 
			curr_child->b_is_dead == best_child->b_is_dead) {
			best_branches.push_back(c);
		}
	}
	if (best_branches.size() > 1) {
		// when we have more than one best-branch, pick one randomly
		cout << "randomly choosing a branch among " 
			 << best_branches.size() << " branches: " 
			 << "was: " << best_branch << " - ";
		best_branch = choice(&best_branches);
		cout << "is now: " << best_branch << endl;
	}
	
	Action best_act = (*p_search_agent->p_game_settings->pv_possible_actions)[best_branch];
	return best_act;
}


/* *********************************************************************
	Moves the best sub-branch of the root to be the new root of the tree
 ******************************************************************* */
void SearchTree::move_to_best_sub_branch(void) {
	assert(p_root->v_children.size() > 0);
	assert(p_root->i_best_branch != -1);
	for (int del = 0; del < p_root->v_children.size(); del++) {
		if (del != p_root->i_best_branch) {
			delete_branch(p_root->v_children[del]);
		}
	}
	TreeNode* old_root = p_root;
	p_root = p_root->v_children[p_root->i_best_branch];
	// make sure the child I want to become root doesn't get deleted:
	old_root->v_children[old_root->i_best_branch] = NULL;
	delete old_root;
	p_root->p_parent = NULL;
}


/* *********************************************************************
	Deletes a node and all its children, all the way down the branch
 ******************************************************************* */
void SearchTree::delete_branch(TreeNode* node) {
	if (!node->v_children.empty()) {
		for(int c = 0; c < node->v_children.size(); c++) {
			delete_branch(node->v_children[c]);
		}
	}
	delete node;
}


/* *********************************************************************
		Prints the Search-Tree, starting from the given node
		if node is NULL (default), we will start from the root
 ******************************************************************* */
void SearchTree::print(TreeNode* node) const {
	if (node == NULL) {
		node = p_root;
	}
	queue<TreeNode*> q;
	q.push(node);
	int curr_level = 0;
	while(!q.empty()) {
		TreeNode* node = q.front();
		q.pop();
		if (node->i_frame_num > curr_level) {
			curr_level = node->i_frame_num;
			cout << endl << curr_level << ": ";
		}
		cout << "(" << node->b_is_dead << ", " << node->f_branch_reward << "),";
		for (unsigned int c = 1; c < node->v_children.size(); c++) {
			q.push(node->v_children[c]);
		}
	}
	cout << endl;
}
