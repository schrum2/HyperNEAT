#ifndef LOGISTIC_H
#define LOGISTIC_H

#include "OSystem.hxx"
#include <deque>
#include <stdio.h>
#include <sstream>
#include <limits>
#include <fstream>
#include <iterator>
#include <boost/thread.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/valarray.hpp>
#include <boost/lexical_cast.hpp>
#include <time.h>
#include <crfsuite.h>
#include <crfsuite_api.hpp>
#include "model.h"
#include "block_grid.h"
#include "common_constants.h"
#include "random_tools.h"

/* Splits a string into a vector of strings. */
static vector<string> split(string s) {
  istringstream iss(s);
  vector<string> tokens;
  copy(istream_iterator<string>(iss),
       istream_iterator<string>(),
       back_inserter<vector<string> >(tokens));
  return tokens;
};

/* Check if a file exists. */
static bool fexists(const char *filename) {
  ifstream ifile(filename);
  return ifile;
};

/* Prints a ValArray in a readable form */
static string printValarray(IntArr a) {
  stringstream s;
  s << "[";
  for (int i=0; i < a.size(); ++i) {
    s << a[i];
    if (i < a.size() -1)
      s << ",";
  }
  s << "]";
  return s.str();
};

/* A basic boolean based pixel feature. Activates if a pixel is a certain color at a certain time. */
struct feature {
  /* friend std::ostream & operator<<(std::ostream &os, const feature &f); */
  friend class boost::serialization::access;

  int x;
  int y;
  int t;
  IntArr color;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* file_version */){
    ar & x & y & t & color;
  };

  string toString(int indx) {
    stringstream s;
    s << "f" << indx << "(" << x << "," << y << "," << printValarray(color) << ")";
    return s.str();
  }
};
/* std::ostream & operator<<(std::ostream &os, const feature &f) */
/* { */
/*     return os << ' ' << gp.degrees << (unsigned char)186 << gp.minutes << '\'' << gp.seconds << '"'; */
/* } */

class treenode {
public:
  string attribute;
  vector<string> values;
  vector<treenode*> children;
  bool terminal;
  int output;
  treenode* parent;

  treenode() {};

  treenode(treenode* _parent) {
    attribute = "";
    terminal = false;
    output = -1;
    parent = _parent;
  };

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* file_version */){
    ar & attribute & values & children & terminal & output & parent;
  };

  void printNode() {
    if (terminal) {
      cout << "Terminal with output value: " << output << endl;
      return;
    }
    cout << attribute << " Attributes[";
    for (int i=0; i<values.size(); ++i)
      cout << values[i] << ", ";
    cout << "] Children[";
    for (int i=0; i<children.size(); ++i)
      cout << children[i] << ", ";
    cout << "]" << endl;
  };

  int matchValue(treenode* curr_node, string desired) {
    for (int i=0; i<curr_node->values.size(); ++i) 
      if (desired.compare(curr_node->values[i]) == 0)
        return i;
    return -1;
  }

  int predict(vector<bool>& f_vals, Action action, Action old_action) {
    assert(parent == NULL);
    assert(terminal || values.size() > 0);
    assert(children.size() == values.size());
    treenode *curr_node = this;
    while (!curr_node->terminal) {
      string desired;
      if (curr_node->attribute[0] == 'f') { // Feature Node
        string s_num = curr_node->attribute.substr(1,curr_node->attribute.size()-1);
        int feat_num;
        istringstream(s_num) >> feat_num;
        if (feat_num >= f_vals.size())
          desired = "false:";
        else
          desired = f_vals[feat_num] ? "true:" : "false:";
      } else if (curr_node->attribute.compare("actionOLD") == 0) { // Action Node
        desired = boost::lexical_cast<string>(old_action) + ":";        
      } else if (curr_node->attribute.compare("action") == 0) { // Action Node
        desired = boost::lexical_cast<string>(action) + ":";
      } else {
        printf("Unexpected node attribute: %s\n",curr_node->attribute.c_str());
        exit(-1);
      }
      int child_indx = matchValue(curr_node,desired);
      if (child_indx < 0)
        curr_node->printNode();
      assert(child_indx >= 0);
      curr_node = curr_node->children[child_indx];
    }
    return curr_node->output;
  };
};


/* The Regression Classifier is used to predict pixel colors. */
struct RegClassifier {
  friend class boost::serialization::access;
  
  vector<vector<float> > lambda_on;    // Weights associated with active features
  vector<vector<float> > lambda_off;   // Weights associated with inactive features
  vector<vector<float> > action_on;    // Weights associated with active actions
  vector<vector<float> > action_off;   // Weights associated with inactive actions
  vector<vector<float> > lambda_trans; // Weights associated with transitions
  vector<IntArr> outputVals;
  int x, y, num_feats, num_labels, numPossibleActions;
  float initial_lambda_val;
  ActionVect pv_possible_actions;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* file_version */){
    ar & lambda_on & lambda_off & action_on & action_off & lambda_trans & outputVals
      & x & y & num_feats & num_labels & numPossibleActions & initial_lambda_val &
      pv_possible_actions;
  };

  RegClassifier() {};

  RegClassifier(int _x, int _y, ActionVect* _pv_possible_actions) {
    x = _x; y = _y;
    pv_possible_actions = (*_pv_possible_actions);
    numPossibleActions = _pv_possible_actions->size();
    num_feats = 0;
    num_labels = 0;
    initial_lambda_val = 0.0;
  };

  void printWeights() {
    for (unsigned y=0; y<num_labels; ++y) {
      for (unsigned k=0; k<num_feats; ++k) {
        printf("f[%d]=0 --> %s : %f\n",k,printValarray(outputVals[y]).c_str(),lambda_off[y][k]);
        printf("f[%d]=1 --> %s : %f\n",k,printValarray(outputVals[y]).c_str(),lambda_on[y][k]);        
      }
      for (unsigned a=0; a<numPossibleActions; ++a) {
        printf("a[%d]=0 --> %s : %f\n",a,printValarray(outputVals[y]).c_str(),action_off[y][a]);
        printf("a[%d]=1 --> %s : %f\n",a,printValarray(outputVals[y]).c_str(),action_on[y][a]);        
      }
    }
  };

  // Checks whether we have observed a given output value
  bool hasSeenColor(IntArr& color) {
    for (unsigned y=0; y<num_labels; ++y)
      if ((color == outputVals[y]).min() == true)
        return true;
    return false;
  };


  // Gets the index in outputVals of a specific color or -1 if not present
  int getOutputIndx(IntArr& color) {
    for (unsigned y=0; y<num_labels; ++y)
      if ((color == outputVals[y]).min() == true)
        return y;
    return -1;
  };

  // Adds new color to our list of observed colors (output vals)
  void addObservedColor(IntArr& color, int numFeats) {
    num_labels++;
    IntArr a(color);
    outputVals.push_back(a);
    lambda_on.push_back(vector<float>());
    lambda_off.push_back(vector<float>());
    for (int y=0; y<lambda_trans.size(); ++y)
      lambda_trans[y].push_back(-numeric_limits<float>::max());
    lambda_trans.push_back(vector<float>());
    for (int y=0; y<num_labels; ++y)
      lambda_trans[num_labels-1].push_back(-numeric_limits<float>::max());
    for (int k=0; k<numFeats; ++k) {
      lambda_on[lambda_on.size()-1].push_back(initial_lambda_val);
      lambda_off[lambda_off.size()-1].push_back(initial_lambda_val);      
    }
    action_on.push_back(vector<float>());
    action_off.push_back(vector<float>());
    for (int a=0; a<numPossibleActions; ++a) {
      action_on[action_on.size()-1].push_back(initial_lambda_val);
      action_off[action_off.size()-1].push_back(initial_lambda_val);
    }
  };

  // Adds a new lambda to correspond to a new feature
  void addNewLambda() {
    num_feats++;
    for (unsigned y=0; y<num_labels; ++y) {
      lambda_on[y].push_back(initial_lambda_val);
      lambda_off[y].push_back(initial_lambda_val);
    }
  };


  // Predicts the next color this pixel will output
  IntArr& predict(vector<bool>& f_vals, Action action, IntArr &prev_label) {
    float maxProb = -numeric_limits<float>::max();
    int maxProbIndx = -1;
    for (unsigned y=0; y<num_labels; ++y) {
      float prob = 0.0;
      for (unsigned k=0; k<num_feats; ++k) {
        if (f_vals[k])
          prob += lambda_on[y][k];
        else
          prob += lambda_off[y][k];
      }
      for (unsigned a=0; a<numPossibleActions; ++a) {
        if (pv_possible_actions[a] == action)
          prob += action_on[y][a];
        else
          prob += action_off[y][a];
      }
      if (prob > maxProb) {
        maxProb = prob;
        maxProbIndx = y;
      }
    }
    return outputVals[maxProbIndx];
  };


  void visualize(vector<feature> feats) {
    for (int y = 0; y < num_labels; ++y) {
      printValarray(outputVals[y]);
      for (int i = 0; i < feats.size(); i++) {
        cout << "\t lambda: " << lambda_on[y][i] << "\tfeature: (" << feats[i].x << "," << feats[i].y << "," << printValarray(feats[i].color) << ") num: " << (y * feats.size() + i) << endl;
      }
    }
  };
};

using namespace CRFSuite;
class LoggingTrainer : public Trainer {
 public:
  void message(const std::string& msg) { cout << msg << endl; };
};


class Logistic : public Model {
 public:
  Logistic(int width, int height, ActionVect* pv_possible_actions, OSystem* p_osystem);
  ~Logistic() {};

  void predict(IntArr* state, Action action, float* reward, IntArr* nextState);
  void update(IntArr* state, Action action, float reward, IntArr* nextState);
  Logistic* clone() const { return new Logistic(*this); };
  void declone() { delete this; };
  void runOptimization();

 protected:
  // Saving and loading by Boost!
  string save_filename;
  void saveState(string filename);
  void loadState(string filename);
  
  void computeFeatureValues(BlockGrid& curr_grid); // Computes the value of each feature
  void crfsuiteOptimize(int x, int y); // Trains a given classifier
  void threadedOptimize(int threadNum, int numThreads); // Threaded opt function
  void printFeatureInfo(int feat_num); // Provides info about a given feature
  void parseWeightsFromFile(string filename, RegClassifier *c); // Parses crfsuite weights from model file
  float evaluateClassifier(int x, int y); // Evaluates the performance of a classifier on training data

  int width, height;
  bool debug; 
  int numColors;
  int maxHistoryLen; // Length of history maintained
  int numPossibleActions; // Number of possible actions for the current game
  ActionVect pv_possible_actions;
  
  vector<feature> f;   // Feature vector
  vector<bool> f_vals; // Current feature values

  // Weights[x,y,v,f] = weight for classifier x,y with output value v for feature f
  vector<vector<RegClassifier> > classifiers;

  // History of feature activations and states.
  // hist[0] is oldest and hist[size] is newest
  deque<vector<bool> > fValHist; // Corresponds to x (observations)
  deque<Action> actionHist;      // History of the taken actions
  deque<BlockGrid> stateHist;    // Corresponds to y (class labels)

  IntArr pv_tmp_color_bits;

  void exportARFF(int x, int y, string filename);
  void exportC45(int x, int y);

  void createDecisionTree(int x, int y); // Calls Quinlans R8 decision tree package
  void parseDecisionTree(ifstream* f, treenode *t, map<string,treenode*> *subtrees);
  void evaluateDecisionTree(int x, int y);
  void printDecisionTree(treenode* t, int depth);
  vector<vector<treenode*> > forest; // Collection of decision trees
};

#endif
