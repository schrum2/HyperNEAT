#include "block_grid.h"
#include "common_constants.h"

BlockGrid::BlockGrid(IntArr* pv_feature_vec, int width, int height, int numColors):grid(*pv_feature_vec) {
  this->width = width;
  this->height = height;
  this->numColors = numColors;
}

BlockGrid::~BlockGrid() {}

// Check if an x,y index is out of range
bool BlockGrid::outOfRange(int x, int y) {
  return x < 0 || x >= width || y < 0 || y >= height;
}

void BlockGrid::getColors(int x, int y, IntArr* pv_tmp_color_bits) {
  if (outOfRange(x,y)) return;
  int startInd = width * numColors * y + numColors * x;
  for (int i = 0; i < numColors; i++) 
    (*pv_tmp_color_bits)[i] = grid[startInd+i];
}

void BlockGrid::getColors(int x, int y, bool b[]) {
  if (outOfRange(x,y)) return;
  int startInd = width * numColors * y + numColors * x;
  for (int i = 0; i < numColors; i++) 
    b[i] = grid[startInd+i];
}

void BlockGrid::setColors(int x, int y, IntArr* new_colors) {
  if (outOfRange(x,y)) return;
  int startInd = width * numColors * y + numColors * x;
  for (int i = 0; i < numColors; i++)
    grid[startInd+i] = (*new_colors)[i];
}

void BlockGrid::setColors(int x, int y, bool b[]) {
  if (outOfRange(x,y)) return;
  int startInd = width * numColors * y + numColors * x;
  for (int i = 0; i < numColors; i++)
    grid[startInd+i] = b[i];
}

void BlockGrid::addColor(int x, int y, int colorIndx) {
  if (outOfRange(x,y)) return;
  grid[width * numColors * y + numColors * x + colorIndx] = 1;
}

void BlockGrid::removeColor(int x, int y, int colorIndx) {
  if (outOfRange(x,y)) return;
  grid[width * numColors * y + numColors * x + colorIndx] = 0;  
}

bool BlockGrid::hasAnyColor(int x, int y) {
  if (outOfRange(x,y)) return false;
  int startInd = width * numColors * y + numColors * x;
  for (int i = 0; i < numColors; i++)
    if (grid[startInd+i])
      return true;
  return false;
}

bool BlockGrid::hasColor(int x, int y, int colorIndx) {
  if (outOfRange(x,y)) return false;
  return grid[width * numColors * y + numColors * x + colorIndx];  
}

void BlockGrid::getIndexes(int colorIndx, IntVect* vec) {
  vec->clear();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (grid[width * numColors * y + numColors * x + colorIndx]) {
        vec->push_back(x);
        vec->push_back(y);
      }
    }
  }
}

float BlockGrid::getSimilarity(BlockGrid* other) {
  assert(height == other->height);
  assert(width == other->width);

  int matchingPixels = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int startInd = width * numColors * y + numColors * x;
      bool match = true;
      for (int i = 0; i < numColors; i++) {
        if (grid[startInd+i] != other->grid[startInd+i]) {
          match = false;
          break;
        }
      }
      if (match)
        matchingPixels++;
    }
  }
  return matchingPixels / (float) (width * height);
}


