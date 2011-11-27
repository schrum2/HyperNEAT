#include "basic_dbn.h"
#include <sstream>
#include <iterator>

BasicDBN::BasicDBN(int width, int height) :
  pv_tmp_color_bits(0,numColors),
  cpt()
{
  this->width = width;
  this->height = height;

  // Initialize Dep
  for (int x = 0; x < width; x++) {
    dep.push_back(vector<vector<coord> >());
    for (int y = 0; y < height; y++) {
      dep[x].push_back(vector<coord>());
    }
  }

  // Provide structure of problem
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (x == 0) { // Left Edge Case
        if (y < 10) {
          dep[x][y].push_back(coord(x,y));          
          dep[x][y].push_back(coord(x+1,y));          
          dep[x][y].push_back(coord(x+2,y));                    
        } else {
          for (int i = 0; i < width; i++)
            dep[x][y].push_back(coord(i,y));
        }
        getSynchronicDeps(x,y);
      } else if (x == width-1) { // Right edge case
        if (y < 10) {
          for (int i = 0; i < width; i++)
            dep[x][y].push_back(coord(i,y));
        } else {
          dep[x][y].push_back(coord(x-2,y));          
          dep[x][y].push_back(coord(x-1,y));          
          dep[x][y].push_back(coord(x,y));          
        }
        getSynchronicDeps(x,y);
      } else if (y >= 2 && y < 10) { // Top Middle
        dep[x][y].push_back(coord(x-1,y));
        dep[x][y].push_back(coord(x,y));
        dep[x][y].push_back(coord(x+1,y));
        if (x+2 < width)
          dep[x][y].push_back(coord(x+2,y));
        getSynchronicDeps(x,y);
      } else if (y >= 10 && y <=17) { // Bottom Middle
        if (x-2 >= 0)
          dep[x][y].push_back(coord(x-2,y));
        dep[x][y].push_back(coord(x-1,y));
        dep[x][y].push_back(coord(x,y));
        dep[x][y].push_back(coord(x+1,y));
        getSynchronicDeps(x,y);
      }

      // Chicken column
      // if (x == 4) {
      //   dep[x][y].push_back(coord(x,y));
      //   if (y > 0)
      //     dep[x][y].push_back(coord(x,y-1));
      //   if (y < height-1)
      //     dep[x][y].push_back(coord(x,y+1));
      // }
    }
  }
}

void BasicDBN::getSynchronicDeps(int x, int y) {
  if (y == 2 || y == 4 || y == 7 || y == 10 || y == 12 || y == 15)
    dep[x][y].push_back(coord(x,y+1,true));
  if (y == 3 || y == 5 || y == 8 || y == 11 || y == 13 || y == 16)
    dep[x][y].push_back(coord(x,y-1,true));
}

void BasicDBN::predict(IntArr* state, Action action, float* reward, IntArr* _nextState) {
  printCPT(width-4,5);
  BlockGrid curr_grid = BlockGrid(state, width, height, numColors);
  (*_nextState) = (*state);
  BlockGrid next_grid = BlockGrid(_nextState, width, height, numColors);

  vector<coord> deps;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      deps = dep[x][y];
      stringstream ss;
      for (int i = 0; i < deps.size(); i++) {
        int dep_x = deps[i].x;
        int dep_y = deps[i].y;

        if (deps[i].synchronic) {
          if (dep_y < y || (dep_y == y && dep_x < x)) { // synch dep computed
            next_grid.getColors(dep_x,dep_y,&pv_tmp_color_bits);
            for (int j = 0; j < pv_tmp_color_bits.size(); j++)
              ss << pv_tmp_color_bits[j];
            ss << "-";
          }
          continue;
        }

        curr_grid.getColors(dep_x,dep_y,&pv_tmp_color_bits);
        for (int j = 0; j < pv_tmp_color_bits.size(); j++)
          ss << pv_tmp_color_bits[j];
        ss << ".";
      }
      cpt_key k = {x,y,ss.str()};
      if (x == 12 && y == 5)
        cout << "QStr:" << ss.str() << endl;
      bool b[8];
      cpt[k].predictOutcome(b);
      next_grid.setColors(x,y,b);
    }
  }

  (*_nextState) = next_grid.grid;
}

void BasicDBN::update(IntArr* state, Action action, float reward, IntArr* nextState) {
  BlockGrid curr_grid = BlockGrid(state,width,height,numColors);
  BlockGrid next_grid = BlockGrid(nextState,width,height,numColors);

  // Check our dependencies
  vector<coord> deps;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      deps = dep[x][y];
      stringstream ss;
      stringstream ss_synchronic;
      for (int i = 0; i < deps.size(); i++) {
        int dep_x = deps[i].x;
        int dep_y = deps[i].y;

        if (deps[i].synchronic) {
          next_grid.getColors(dep_x,dep_y,&pv_tmp_color_bits);
          for (int j = 0; j < pv_tmp_color_bits.size(); j++)
            ss_synchronic << pv_tmp_color_bits[j];
          ss_synchronic << "-";
          continue;
        } 

        curr_grid.getColors(dep_x,dep_y,&pv_tmp_color_bits);
        for (int j = 0; j < pv_tmp_color_bits.size(); j++) {
          ss << pv_tmp_color_bits[j];
          ss_synchronic << pv_tmp_color_bits[j];
        }
        ss << ".";
        ss_synchronic << ".";
      }
      cpt_key k = {x,y,ss.str()};
      bool b[8];
      next_grid.getColors(x,y,b);
      outcome o(b);
      cpt[k].addObservedOutcome(o);
      if (ss.str() != ss_synchronic.str()) {
        cpt_key syn_k = {x,y,ss_synchronic.str()};
        cpt[syn_k].addObservedOutcome(o);
      }
    }
  }
}

void BasicDBN::printCPT(int x, int y) {
  map<cpt_key,cpt_value>::const_iterator iter;
  for (iter=cpt.begin(); iter!=cpt.end(); iter++) {
    if (iter->first.x == x && iter->first.y == y)
      cout << "key: " << iter->first << " value: " << iter->second << endl;
  }
}
