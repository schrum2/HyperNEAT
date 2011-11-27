/* *****************************************************************************
 * A.L.E (Atari 2600 Learning Environment)
 * Copyright (c) 2009-2010 by Yavar Naddaf
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************/

#include "node.h"

Node::Node(Node* parent):
  p_parent(parent),
  f_branch_reward(0.0),
  i_best_branch(-1), 
  f_uct_value(0.0),
  i_uct_visit_count(0),
  f_uct_sum_reward(0.0)
  {
    if (parent == NULL)
      depth = 0;
    else
      depth = parent->depth + 1;
  }

