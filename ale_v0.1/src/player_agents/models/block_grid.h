#ifndef BLOCK_GRID_H
#define BLOCK_GRID_H

#include "../common_constants.h"

class BlockGrid {
 public:
  BlockGrid(IntArr* pv_feature_vec, int width, int height, int numColors);
  virtual ~BlockGrid();

  bool outOfRange(int x, int y);
  void getColors(int x, int y, IntArr* pv_tmp_color_bits);
  void getColors(int x, int y, bool b[]);
  void setColors(int x, int y, IntArr* new_colors);
  void setColors(int x, int y, bool b[]);
  void addColor(int x, int y, int colorIndx);
  void removeColor(int x, int y, int colorIndx);
  bool hasAnyColor(int x, int y);
  bool hasColor(int x, int y, int colorIndx);
  void getIndexes(int colorIndx, IntVect* vec);
  float getSimilarity(BlockGrid* other);

  IntArr grid;

 protected:
  int width;
  int height;
  int numColors;
};

#endif
