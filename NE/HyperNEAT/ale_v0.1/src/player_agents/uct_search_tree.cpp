/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  uct_search_tree.cpp
 *
 *  Implementation of the UCTSearchTree class, an implementation of the 
 *  Upper Confidence Bound for Trees algorithm for the Rollout agent
 **************************************************************************** */

#include "uct_search_tree.h"
#include "search_agent.h"
#include "random_tools.h"

/* *********************************************************************
	Constructor
 ******************************************************************* */
UCTSearchTree::UCTSearchTree(SearchAgent* search_agent) :
  SearchTree(search_agent) {
  Settings& settings = p_search_agent->p_osystem->settings();
  i_uct_monte_carlo_steps = settings.getInt("uct_monte_carlo_steps", true);
  cout << "Monte Carlo Steps per iteration: " << i_uct_monte_carlo_steps << endl;
  f_uct_exploration_const = settings.getFloat("uct_exploration_const", true);
  i_uct_min_death_count = settings.getInt("uct_min_death_count", true);
  if (settings.getString("uct_branch_value_method", true) == "average") {
    cout << "UCT: taking the average value of the children" << endl;
    b_branch_value_average = true;
  } else {
    cout << "UCT: taking the max value of the children" << endl;
    b_branch_value_average = false;
  }
  b_avg_reward_per_frame = settings.getBool("uct_avg_reward_per_frame", 
                                            true);
  if (b_avg_reward_per_frame) {
    cout << "UCT: uct_avg_reward_per_frame is true. " << 
      "looking at reward/frame, not just reward" << endl;
  }
}

/* *********************************************************************
   Deconstructor
   ******************************************************************* */
UCTSearchTree::~UCTSearchTree() {

}

/* *********************************************************************
   Builds a new tree
   ******************************************************************* */
void UCTSearchTree::build(const string start_state, int start_frame_num) {
  assert(p_root == NULL);
  p_root = new TreeNode(NULL, start_state, start_frame_num, 
                        p_search_agent, 0, UNDEFINED);
  if (p_root->b_is_dead) {
    cout << "OMG! WE'RE GONNA DIE!! :((   ... frame:" << start_frame_num << endl;
    print();
  }

  expand_node(p_root);
  update_tree();
  is_built = true;
}

/* *********************************************************************
   Re-Expands the tree until i_max_sim_steps_per_tree is reached
   ******************************************************************* */
void UCTSearchTree::update_tree(void) {
  int i = 0;
  while(true) {
    i++;
    single_uct_iteration();
    if (p_search_agent->get_num_simulated_steps() > 
        i_max_sim_steps_per_tree) {
      cout << "Performed " << i << " UCT iterations" << endl;
      break;
    }
  }
}


/* *********************************************************************
   Performs a single UCT iteration, starting from the root
   ******************************************************************* */
void UCTSearchTree::single_uct_iteration(void) {
  TreeNode* curr_node = p_root;

  bool found_visited_node = true;
  while (!curr_node->is_leaf()) {
    // See if this node has any children with count = 0
    int zero_count_child = get_child_with_count_zero(curr_node);
    if (zero_count_child != -1) {
      found_visited_node = false;
      curr_node = curr_node->v_children[zero_count_child];
    } else {
      int best_uct_branch = get_best_branch(curr_node, true);
      curr_node = curr_node->v_children[best_uct_branch];
    }
  }
	
  if (found_visited_node) {
    expand_node(curr_node);
    curr_node = curr_node->v_children[0]; // pick the first child
  }
	
  // Do a manto-carlo search for i_uct_monte_carlo_steps steps 
  float new_reward;
  bool is_dead;
  do_monte_carlo(curr_node, new_reward, is_dead);
  is_dead = is_dead || curr_node->b_is_dead;	// death either during 
  // monte carlo rt node itself
  if (b_avg_reward_per_frame) {
    int frames_from_root = curr_node->i_frame_num - p_root->i_frame_num;
    assert (frames_from_root > 0);
    frames_from_root += i_uct_monte_carlo_steps;
    new_reward /= frames_from_root;
  }
  update_values(curr_node, new_reward, is_dead);
	
}

/* *********************************************************************
   Returns the best action based on the expanded search tree
   ******************************************************************* */
Action UCTSearchTree::get_best_action(void) {
  assert (p_root != NULL);
  int best_branch = get_best_branch(p_root, false);
  TreeNode* best_child = p_root->v_children[best_branch];
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
    cout << "randomly choosing a branch among " << best_branches.size() 
         << " branches. was: " << best_branch << " - ";
    best_branch = choice(&best_branches);
    cout << "is now: " << best_branch << endl;
  }
  p_root->i_best_branch = best_branch;
  Action best_act = (*p_search_agent->p_game_settings->
                     pv_possible_actions)[best_branch];
  return best_act;
}

/* *********************************************************************
   Returns the index of the first child with zero count
   Returns -1 if no such child is found
   ******************************************************************* */
int UCTSearchTree::get_child_with_count_zero(const TreeNode* node)  const {
  for (unsigned int c = 0; c < node->v_children.size(); c++) {
    if (node->v_children[c]->i_uct_visit_count == 0) {
      return c;
    }
  }
  return -1;
}

/* *********************************************************************
   Returns the sub-branch with the highest value
   if add_exp_explt_val is true, we will add the UCT's
   Exploration-Exploitation to each branch value and then take the max
   ******************************************************************* */
int UCTSearchTree::get_best_branch(TreeNode* node, 
                                   bool add_exp_explt_val) {
  float best_value = 0;
  int best_branch = -1;
  if (node->v_children.size() == 0) {
    print();
    cerr << "get_best_branch called on a node with no child. frame-num: " 
         << node->i_frame_num << endl;
    exit(-1);
  }
  bool all_children_dead = true;
  for (unsigned int c = 0; c < node->v_children.size(); c++) {
    all_children_dead = all_children_dead & node->v_children[c]->b_is_dead;
  }
  if (all_children_dead) {
    // it is (very rarely) possible to have all children dead, but the node
    // itself being alive (i.e. if the very first simulation on the node 
    //	didn't die, but all the consequent simulations on its children did
    // end up dead). 
    // Here we fix that: if all your children are dead, you are dead
    node->b_is_dead = true;
  }
  for (unsigned int c = 0; c < node->v_children.size(); c++) {
    TreeNode* curr_child = node->v_children[c];
    if ((!node->b_is_dead) && curr_child->b_is_dead) {
      continue;
    }
    float curr_val = curr_child->f_branch_reward;
    if (add_exp_explt_val) {
      float expr_explt_val = log(	(double)node->i_uct_visit_count) / 
        (double)(curr_child->i_uct_visit_count);
      expr_explt_val = sqrt(expr_explt_val);
      // assert (curr_val < 0 || // i want them in the same Order.of.Mag
      //		(((int)(curr_val + 1) / (int)(expr_explt_val + 1)) < 10 &&	
      //		((int)(expr_explt_val + 1) / (int)(curr_val + 1)) < 10));
      curr_val += f_uct_exploration_const * expr_explt_val;
    }
    if (best_branch == -1 || 
        curr_val > best_value) {
      best_value = curr_val;
      best_branch = c;
    }
  }
  if (best_branch == -1) {
    // we have a bug here :(
    cerr << "Best-branch is -1 for Node with i_frame = " 
         << node->i_frame_num << endl
         << "Printing the tree starting from the given node: " << endl;
    print(const_cast<TreeNode *>(node));
    exit(-1);
  }
  return best_branch;
}


/* *********************************************************************
   Expands the given node, by generating all its children
   ******************************************************************* */
void UCTSearchTree::expand_node(TreeNode* node) {
  assert(node->is_leaf());
  for (int a = 0; a < p_search_agent->i_num_actions; a++) {
    Action act = (*p_search_agent->p_game_settings->pv_possible_actions)[a];
    TreeNode* new_child = new TreeNode(	node,
                                        node->str_state, 
                                        node->i_frame_num, 
                                        p_search_agent, 
                                        i_sim_steps_per_node, 
                                        act);
    node->v_children.push_back(new_child);
    if (new_child->i_frame_num > i_deepest_node_frame_num) {
      i_deepest_node_frame_num = new_child->i_frame_num;
    }
    if (b_avg_reward_per_frame) {
      int frames_from_root = new_child->i_frame_num - p_root->i_frame_num;
      assert (frames_from_root > 0);
      new_child->f_node_reward /= frames_from_root;
      new_child->f_branch_reward /= frames_from_root;
    }
  }
}


/* *********************************************************************
   Performs a Monte Carlo simulation from the given node, for
   i_uct_monte_carlo_steps steps. Returns the reward recieved, and 
   wether the agent died or not
   ******************************************************************* */
void UCTSearchTree::do_monte_carlo(TreeNode* start_node, 
                                   float& reward, bool& is_dead) {
  p_search_agent->load_state(start_node->str_state);
  p_search_agent->simulate_game(RANDOM, i_uct_monte_carlo_steps, 
                                start_node->i_frame_num, 
                                reward, is_dead);

}
/* *********************************************************************
   Update the node values and counters from the current node, all the
   way up to the root
   ******************************************************************* */
void UCTSearchTree::update_values(TreeNode* node, float reward, bool is_dead) {
  while (node != NULL) {
    node->i_uct_visit_count++;
    if (is_dead) { 
      node->i_uct_death_count++;
      if (node->i_uct_death_count >= i_uct_min_death_count &&
          node->i_uct_death_count == node->i_uct_visit_count) {
        // this will probably end up with our death
        node->b_is_dead = true;
      }
    } else {
      node->b_is_dead = false;
    }
		
    if (node->i_uct_visit_count == 1) {	
      node->f_branch_reward = 0;
      node->f_uct_sum_reward = reward;
    } else {
      if (b_branch_value_average) {
        // take the average value of the children
        node->f_uct_sum_reward += reward;
        node->f_branch_reward = 
          node->f_uct_sum_reward / node->i_uct_visit_count;
      } else {
        // take the max value of the children
        if (node->i_uct_death_count < i_uct_min_death_count ||
            node->b_is_dead == is_dead) {
          node->f_branch_reward = max(node->f_branch_reward, 
                                      node->f_node_reward + reward);
        }
      }
    }
    node->f_branch_reward += node->f_node_reward;
    reward = node->f_branch_reward;		// THIS IS NEWLY ADDED!
    node = node->p_parent;
    reward *= f_discount_factor;
  }
}


/* *********************************************************************
   Prints the Search-Tree, starting from the given node
   if node is NULL (default), we will start from the root
   ******************************************************************* */
void UCTSearchTree::print(TreeNode* node) const {
  if (node == NULL) {
    node = p_root;
  }
  queue<TreeNode*> q;
  q.push(node);
  int curr_level = 0;
  cerr << "(i_uct_visit_count, f_branch_reward, i_uct_death_count, " 
       << "b_is_dead)" << endl;
  while(!q.empty()) {
    TreeNode* node = q.front();
    q.pop();
    assert (!(node->i_frame_num < curr_level));
    if (node->i_frame_num > curr_level) {
      curr_level = node->i_frame_num;
      cerr << endl << curr_level << ": ";
    }
    cerr << "(" << node->i_uct_visit_count << "," 
         << node->f_branch_reward << ","
         << node->i_uct_death_count << ","
         << node->b_is_dead <<  "),";
    for (unsigned int c = 0; c < node->v_children.size(); c++) {
      q.push(node->v_children[c]);
    }
  }
  cout << endl;
}
