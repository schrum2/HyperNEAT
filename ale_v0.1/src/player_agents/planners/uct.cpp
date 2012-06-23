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

#include "uct.h"
#include "../../common/random_tools.h"
#include <queue>

/* *********************************************************************
   Constructor
   ******************************************************************* */
UCT::UCT(Settings& settings, ActionVect* possible_actions):
  //p_model_agent(model_agent),
  is_built(false),
  p_root(NULL),
  numNodes(0),
  numDeletedNodes(1),
  numAddedNodes(1)
{
  this->possible_actions = possible_actions;
  //Settings& settings = p_model_agent->p_osystem->settings();
  num_steps_between_nodes = settings.getInt("sim_steps_per_node", true);
  int max_sim_steps_per_frame = settings.getInt("max_sim_steps_per_frame",true);
  max_sim_steps = max_sim_steps_per_frame * num_steps_between_nodes;
  f_discount_factor = settings.getFloat("discount_factor", true);
  max_monte_carlo_steps = settings.getInt("uct_monte_carlo_steps", true);
  cout << "Monte Carlo Steps per iteration: " << max_monte_carlo_steps << endl;
  f_uct_exploration_const = settings.getFloat("uct_exploration_const", true);
  if (settings.getString("uct_branch_value_method", true) == "average") {
    cout << "UCT: taking the average value of the children" << endl;
    b_branch_value_average = true;
  } else {
    cout << "UCT: taking the max value of the children" << endl;
    b_branch_value_average = false;
  }
  b_avg_reward_per_frame = settings.getBool("uct_avg_reward_per_frame", true);
  if (b_avg_reward_per_frame) {
    cout << "UCT: uct_avg_reward_per_frame is true. " << 
      "looking at reward/frame, not just reward" << endl;
  }
}

/* *********************************************************************
   Deconstructor
   ******************************************************************* */
UCT::~UCT() {
  clear();
}

/* *********************************************************************
   Deletes the search-tree
   ******************************************************************* */
void UCT::clear(void) {
  if (p_root != NULL) {
    delete_branch(p_root);
    p_root = NULL;
  }
  is_built = false;
}

/* *********************************************************************
   Moves the best sub-branch of the root to be the new root of the tree
   ******************************************************************* */
void UCT::move_to_best_sub_branch(void) {
  assert(p_root->v_children.size() > 0);
  assert(p_root->i_best_branch != -1);
  for (unsigned del = 0; del < p_root->v_children.size(); del++) {
    if (del != p_root->i_best_branch) {
      delete_branch(p_root->v_children[del]);
    }
  }
  Node* old_root = p_root;
  p_root = p_root->v_children[p_root->i_best_branch];
  // make sure the child I want to become root doesn't get deleted:
  old_root->v_children[old_root->i_best_branch] = NULL;
  delete old_root;
  p_root->p_parent = NULL;
}


/* *********************************************************************
   Deletes a node and all its children, all the way down the branch
   ******************************************************************* */
void UCT::delete_branch(Node* node) {
  if (!node->v_children.empty()) {
    for(unsigned c = 0; c < node->v_children.size(); c++) {
      delete_branch(node->v_children[c]);
    }
  }
  numDeletedNodes++;
  delete node;
}


/* *********************************************************************
   Builds a new tree
   ******************************************************************* */
void UCT::build(const Model& model, const IntArr& current_state) {
  assert(p_root == NULL);
  p_root = new Node(NULL);
  numAddedNodes++;
  expand_node(p_root);
  update_tree(model, current_state);
  is_built = true;
}

/* *********************************************************************
   Expands the given node, by generating all its children
   ******************************************************************* */
void UCT::expand_node(Node* node) {
  assert(node->is_leaf());
  for (unsigned a = 0; a < possible_actions->size(); a++) {
    //Action act = (*p_model_agent->p_game_settings->pv_possible_actions)[a];
    Node* new_child = new Node(node);
    node->v_children.push_back(new_child);
    numAddedNodes++;
  }
}


/* *********************************************************************
   Re-Expands the tree until i_max_sim_steps_per_tree is reached. This
   is the main entry point for running a UCT simulation.
   ******************************************************************* */
void UCT::update_tree(const Model& model, const IntArr& current_state) {
  int i = 0;
  curr_sim_steps = 0;

  assert(numDeletedNodes > 0);
  while (curr_sim_steps < max_sim_steps) {
    i++;
    single_uct_iteration(model, current_state);
  }
  
  numNodes += numAddedNodes - numDeletedNodes;

  cout << "Total Nodes: " << numNodes << " Added: " << numAddedNodes << " Deleted: " << numDeletedNodes << endl;

  numAddedNodes = 0;
  numDeletedNodes = 0;
  
  cout << "Performed " << i << " UCT iterations before reaching " << max_sim_steps << " sim steps." << endl;
}

// Performs a single UCT iteration, starting from the root
void UCT::single_uct_iteration(const Model& model, const IntArr& start_state) {
  Model* curr_model = model.clone();
  IntArr curr_state(0,start_state.size());
  curr_state = start_state;
  IntArr next_state(0,start_state.size());
  float reward = 0.0;
  Node* curr_node = p_root;

  bool found_visited_node = true;
  while (!curr_node->is_leaf()) {
    // See if this node has any children with count = 0
    int zero_count_child = get_child_with_count_zero(curr_node);
    if (zero_count_child != -1) {
      found_visited_node = false;
      sim_step(curr_model, zero_count_child, reward, curr_state, next_state);
      curr_node = curr_node->v_children[zero_count_child];
    } else {
      int best_uct_branch = get_best_branch(curr_node, true);
      sim_step(curr_model, best_uct_branch, reward, curr_state, next_state);
      curr_node = curr_node->v_children[best_uct_branch];
    }
  }
	
  if (found_visited_node) {
    expand_node(curr_node);
    curr_node = curr_node->v_children[0]; // pick the first child
  }
	
  // Do a manto-carlo search for i_uct_monte_carlo_steps steps 
  float playout_reward;
  do_monte_carlo(curr_node, playout_reward, curr_model, curr_state, next_state);
  update_values(curr_node, playout_reward);
  curr_model->declone();
}

// Take a single simulated step
void UCT::sim_step(Model* curr_model, int actionIndx, float& reward, IntArr& curr_state, IntArr& next_state) {
  Action a = (*possible_actions)[actionIndx];
  curr_model->predict(&curr_state, a, &reward, &next_state);
  curr_model->update(&curr_state, a, reward, &next_state);
  curr_state = next_state;
  curr_sim_steps++;
}

// Take a single simulated step
void UCT::sim_step(Model* curr_model, Action a, float& reward, IntArr& curr_state, IntArr& next_state) {
  curr_model->predict(&curr_state, a, &reward, &next_state);
  curr_model->update(&curr_state, a, reward, &next_state);
  curr_state = next_state;
  curr_sim_steps++;
}

// Performs a Monte Carlo playout from the given node
void UCT::do_monte_carlo(Node* start_node, float& reward, Model* model, IntArr& curr_state, IntArr& next_state) {
  reward = 0.0;
  float step_reward = 0.0;
  for (int i = 0; i < max_monte_carlo_steps; i++) {
    Action a = choice(possible_actions);
    sim_step(model, a, step_reward, curr_state, next_state);
    reward += step_reward;
  }
}

// Returns the best action based on the expanded search tree
Action UCT::get_best_action(void) {
  assert (p_root != NULL);
  int best_branch = get_best_branch(p_root, false);
  Node* best_child = p_root->v_children[best_branch];
  vector<int> best_branches;
  best_branches.push_back(best_branch);
  for (unsigned c = 0; c < p_root->v_children.size(); c++) {
    Node* curr_child = p_root->v_children[c];
    if (c != best_branch && curr_child->f_branch_reward == best_child->f_branch_reward) {
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
  Action best_act = (*possible_actions)[best_branch];
  return best_act;
}

// Returns the index of the first child with zero count. Returns -1 if no such child is found
int UCT::get_child_with_count_zero(const Node* node)  const {
  for (unsigned int c = 0; c < node->v_children.size(); c++) {
    if (node->v_children[c]->i_uct_visit_count == 0) {
      return c;
    }
  }
  return -1;
}

// Returns the sub-branch with the highest value. If add_exp_explt_val is true, we will add the UCT's Exploration-Exploitation to each branch value and then take the max
int UCT::get_best_branch(Node* node, bool add_exp_explt_val) {
  float best_value = 0;
  int best_branch = -1;
  if (node->v_children.size() == 0) {
    print();
    cerr << "get_best_branch called on a node with no child." << endl;
    exit(-1);
  }
  for (unsigned int c = 0; c < node->v_children.size(); c++) {
    Node* curr_child = node->v_children[c];
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
    cerr << "Best-branch is -1 for Node." << endl
         << "Printing the tree starting from the given node: " << endl;
    print(const_cast<Node *>(node));
    exit(-1);
  }
  return best_branch;
}

// Update the node values and counters from the current node, all the way up to the root
void UCT::update_values(Node* node, float playout_reward) {
  while (node != NULL) {
    node->i_uct_visit_count++;
    // take the max value of the children
    node->f_branch_reward = max(node->f_branch_reward, playout_reward);
    //reward = node->f_branch_reward;		// THIS IS NEWLY ADDED!
    node = node->p_parent;
    playout_reward *= f_discount_factor;
  }
}

/* *********************************************************************
   Prints the Search-Tree, starting from the given node
   if node is NULL (default), we will start from the root
   ******************************************************************* */
void UCT::print(Node* node) const {
  if (node == NULL) {
    node = p_root;
  }
  queue<Node*> q;
  q.push(node);
  int curr_level = 0;
  cerr << "(i_uct_visit_count, f_branch_reward, i_uct_death_count, " 
       << "b_is_dead)" << endl;
  while(!q.empty()) {
    Node* node = q.front();
    q.pop();
    assert (!(node->depth < curr_level));
    if (node->depth > curr_level) {
      curr_level = node->depth;
      cerr << endl << curr_level << ": ";
    }
    cerr << "(" << node->i_uct_visit_count << "," 
         << node->f_branch_reward << "),";
    for (unsigned int c = 0; c < node->v_children.size(); c++) {
      q.push(node->v_children[c]);
    }
  }
  cout << endl;
}
