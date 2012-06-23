#ifndef BASIC_DBN_H
#define BASIC_DBN_H

#include "model.h"
#include "block_grid.h"
#include "../common_constants.h"
#include <map>

struct floatArr {
  float values[8];
};

// 2D Coordinate
struct coord {
  int x;
  int y;
  bool synchronic;
coord(int _x, int _y) : x(_x), y(_y), synchronic(false) { };
coord(int _x, int _y, bool _synchronic) : x(_x), y(_y), synchronic(_synchronic) { };
};

struct outcome {
  bool b[8];

  outcome(bool _b[]) {
    for (int i = 0; i < 8; i++)
      b[i] = _b[i];
  };

  bool operator==(const outcome& other) const {
    for (int i = 0; i < 8; i++)
      if (b[i] != other.b[i])
        return false;
    return true;
  };

  friend ostream& operator<<(ostream &stream, outcome o) {
    stream << "(";
    for (int i = 0; i < 8; i++)
      stream << o.b[i];
    stream << ")";
    return stream;
  };

  bool operator<(const outcome& other) const {
    for (int i = 0; i < 8; i++) {
      if (b[i] > other.b[i])
        return false;
      if (b[i] < other.b[i])
        return true;
    }
    return false;
  };
};

struct cpt_key {
  int x;
  int y;
  string s;

  bool operator<(const cpt_key& other) const {
    return x < other.x || (x == other.x && (y < other.y || (y == other.y && s < other.s)));
  };

  friend ostream& operator<<(ostream &stream, cpt_key k) {
    stream << "(" << k.x << "," << k.y << ") - s:" << k.s;
    return stream;
  };
};

struct cpt_value {
  vector<outcome> outcomes;
  vector<int> counts;
  int total;
  cpt_value() : outcomes(), counts(), total(0) {};

  void addObservedOutcome(outcome& o) {
    bool found = false;
    for (u_int i = 0; i < outcomes.size(); i++) {
      if (outcomes[i] == o) {
        counts[i]++;
        found = true;
        break;
      }
    }
    if (!found) {
      outcomes.push_back(o);
      counts.push_back(1);
    }
    total++;
  };

  void predictOutcome(bool b[]) {
    if (total == 0) {
      cout << "Unexpected key." << endl;
      for (int i = 0; i < 8; i++)
        b[i] = false;
    } else {
      int v = rand() % total;
      int index = 0;
      int cumulative = 0;
      do {
        cumulative += counts[index];
        index++;
      } while (cumulative < v);
      outcome o = outcomes[index-1];
      for (int i = 0; i < 8; i++)
        b[i] = o.b[i];
    }
  };

  friend ostream& operator<<(ostream &stream, cpt_value v) {
    for (int i = 0; i < v.outcomes.size(); i++) {
      float prob = v.counts[i] / (float)v.total;
      stream << v.outcomes[i] << " : " << prob << ", ";
    }
    stream << "Total " << v.total;
    return stream;
  };
};

class BasicDBN : public Model {
 public:
  BasicDBN(int width, int height);
  ~BasicDBN() {};

  // Given state and action, the model will predict reward and next state.
  void predict(IntArr* state, Action action, float* reward, IntArr* nextState);
  // Update the model with an experience tuple
  void update(IntArr* state, Action action, float reward, IntArr* nextState);
  BasicDBN* clone() const { return new BasicDBN(*this); };
  void declone() { delete this; };
  void printCPT(int x, int y);

 protected:
  void getSynchronicDeps(int x, int y);

  int width, height;
  static const int numColors = 8;
  IntArr pv_tmp_color_bits;


  map<cpt_key,cpt_value> cpt; // Conditional probability table
  vector<vector<vector<coord> > > dep; // Dependencies
};

#endif
