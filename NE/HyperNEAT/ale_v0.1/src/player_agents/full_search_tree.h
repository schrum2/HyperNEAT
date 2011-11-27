/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  full_search_tree.h
 *
 *  Implementation of the FullSearchTree class, which represents search a
 *  Full-Search-Tree (i.e. a search-tree taht expands all its nodes until
 *  the give ndebth), used by the Search Agent
 **************************************************************************** */

#ifndef FULL_SEARCH_TREE_H
#define FULL_SEARCH_TREE_H

#include "search_tree.h"



class FullSearchTree : public SearchTree {
    /* *************************************************************************
        Represents search a Full-Search-Tree (i.e. a search-tree taht expands 
		all its nodes until the give ndebth), used by the Search Agent
    ************************************************************************* */
	
    public:
		/* *********************************************************************
            Constructor
			Generates a whole search tree from the current state, until 
			max_frame_num is reached.
         ******************************************************************* */
		FullSearchTree(SearchAgent* search_agent);

		/* *********************************************************************
            Deconstructor
         ******************************************************************* */
		virtual ~FullSearchTree();

		/* *********************************************************************
            Builds a new tree
         ******************************************************************* */
		virtual void build(	const string start_state, int start_frame_num);
		
		/* *********************************************************************
			Re-Expands the tree until i_max_sim_steps_per_tree is reached
         ******************************************************************* */
		virtual void update_tree();
		
	protected:	

		/* *********************************************************************
			Expands the tree from the given node until i_max_sim_steps_per_tree
         ******************************************************************* */
		virtual void expand_tree(TreeNode* start);


		/* *********************************************************************
			Updates the branch reward for the given node
			which equals to: node_reward + max(children.branch_reward)
         ******************************************************************* */
		void update_branch_reward(TreeNode* node);


};



#endif
