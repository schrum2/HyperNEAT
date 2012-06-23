#include "logistic.h"
#include <crfsuite.hpp>


Logistic::Logistic(int width, int height, ActionVect* pv_possible_actions, OSystem* p_osystem):
  debug(true),
  numColors(8),
  maxHistoryLen(numeric_limits<int>::max()),
  pv_tmp_color_bits(0,numColors)
{
  this->width = width;
  this->height = height;
  this->numPossibleActions = pv_possible_actions->size();
  this->pv_possible_actions = *pv_possible_actions;

  Settings& settings = p_osystem->settings();
  save_filename = settings.getString("save_model",false);

  for (int x = 0; x < width; x++) {
    classifiers.push_back(vector<RegClassifier>());
    for (int y = 0; y < height; y++)
      classifiers[x].push_back(RegClassifier(x,y,pv_possible_actions));
  }

  // Create our forest of decision trees
  for (int x = 0; x < width; x++) {
    forest.push_back(vector<treenode*>());
    for (int y = 0; y < height; y++)
      forest[x].push_back(new treenode(NULL));
  }

  // Potentially load saved weights
  string load_filename = settings.getString("load_model",true);
  if (!load_filename.empty()) {
    loadState(load_filename);
    cout << "Successfully Loaded " << load_filename << endl;
  }
};

void Logistic::saveState(string filename) {
  ofstream ofs(filename.c_str());
  boost::archive::text_oarchive oa(ofs);
  oa << f; // Save the features
  oa << classifiers;
  oa << forest;
};

void Logistic::loadState(string filename) {
  std::ifstream ifs(filename.c_str());
  boost::archive::text_iarchive ia(ifs);
  // restore the schedule from the archive
  ia >> f;
  ia >> classifiers;
  ia >> forest;
};

/* Predicts the next state of the environment. */
void Logistic::predict(IntArr* state, Action action, float* reward, IntArr* _nextState)
{
  BlockGrid curr_grid = BlockGrid(state, width, height, numColors);
  (*_nextState) = (*state);
  BlockGrid next_grid = BlockGrid(_nextState, width, height, numColors);
  computeFeatureValues(curr_grid);
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      curr_grid.getColors(x,y,&pv_tmp_color_bits);

      // Regression Classifier
      // IntArr prediction = classifiers[x][y].predict(f_vals,action,pv_tmp_color_bits);

      // Decision Tree Classifier
      int prediction_indx = forest[x][y]->predict(f_vals,action,actionHist.back());
      if (prediction_indx == -1) {
        printf("Got neg prediction indx for classifier %d,%d\n",x,y);
        exit(-1);
      }
      IntArr prediction = classifiers[x][y].outputVals[prediction_indx];

      next_grid.setColors(x,y,&prediction);
    }
  }
  (*reward) = 0.0;
  (*_nextState) = next_grid.grid;
};

/* Updates the model with current information about state transitions and rewards. */
void Logistic::update(IntArr* state, Action action, float reward, IntArr* nextState)
{
  BlockGrid curr_grid = BlockGrid(state,width,height,numColors);
  BlockGrid next_grid = BlockGrid(nextState,width,height,numColors);

  computeFeatureValues(curr_grid);
  fValHist.push_back(f_vals);
  stateHist.push_back(next_grid);
  actionHist.push_back(action);
  assert(fValHist.size() == stateHist.size() && actionHist.size() == stateHist.size());
  while (fValHist.size() > maxHistoryLen) {
    fValHist.pop_front();
    stateHist.pop_front();
    actionHist.pop_front();
  }

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      next_grid.getColors(x,y,&pv_tmp_color_bits);
      if (!classifiers[x][y].hasSeenColor(pv_tmp_color_bits)) {
        classifiers[x][y].addObservedColor(pv_tmp_color_bits, f.size());
        // Add to feature vector
        feature feat = {x,y,0,pv_tmp_color_bits};
        f.push_back(feat);
        f_vals.push_back(false);

        // Expand lambdas for all classifiers
        for (int i = 0; i < width; i++)
          for (int j = 0; j < height; j++)
            classifiers[i][j].addNewLambda();
      }
    }
  }
};


/* Computes the values of all of our current features. */
void Logistic::computeFeatureValues(BlockGrid& curr_grid)
{
  while (f_vals.size() < f.size())
    f_vals.push_back(false);

  for (unsigned i = 0; i < f.size(); i++) {
    feature feat = f[i];
    curr_grid.getColors(feat.x,feat.y,&pv_tmp_color_bits);
    f_vals[i] = (pv_tmp_color_bits == feat.color).min() == true;
  }
};

/* Optimizes the parameters for all of our classifiers */
void Logistic::runOptimization() {
  cout << "Running Optimization..." << endl;
  // int testx = 5;
  // int testy = 4;
  // //exportC45(testx, testy);
  // createDecisionTree(testx,testy);
  // printDecisionTree(forest[testx][testy],0);
  // evaluateDecisionTree(testx,testy);
  // exit(0);

  time_t start, end;
  using namespace boost;

  int numThreads = thread::hardware_concurrency();
  thread holder[numThreads];

  cout << "Starting Optimization with " << numThreads << " threads..." << endl;

  time(&start);
  for (int i=0; i<numThreads; ++i)
    holder[i] = thread(&Logistic::threadedOptimize,this,i,numThreads);
  
  for (int i=0; i<numThreads; ++i)
    holder[i].join();

  time(&end);
  double diff = difftime(end,start);
  cout << "Optimization finished in " << diff << " seconds." << endl;

  if (!save_filename.empty()) {
    saveState(save_filename);
    cout << "Successfully saved to " << save_filename << endl;
    exit(-1);
  }
};

/* Optimize function for a single thread. */
void Logistic::threadedOptimize(int threadNum, int numThreads) {
  int jobs = (width * height) / numThreads;
  int start = jobs * threadNum;
  if (threadNum == numThreads-1) // Last thread gets extra work
    jobs = (width * height) - start;
  for (int i=0; i<jobs; ++i) {
    int x = start % width;
    int y = start / width;
    printf("Thread %d starting Opt on classifier (%d,%d)\n",threadNum,x,y);
    exportC45(x,y);
    createDecisionTree(x, y);
    //crfsuiteOptimize(x,y);
    // float accuracy = evaluateClassifier(x,y);
    // if (accuracy < 1.0)
    //   printf("Classifier<Accuracy:%f> ",accuracy);
    // else
    //   cout << "(%d.%d)" << threadNum << " " << flush;
    start++;
  }
};


/* Tests a given classifier's performance on the current state history. */
float Logistic::evaluateClassifier(int x, int y) {
  IntArr colors(0,numColors);
  int correct = 0;
  for (int t=0; t<fValHist.size(); ++t) {
    if (t > 0) 
      stateHist[t-1].getColors(x,y,&colors);
    else
      for (int i=0; i<8; ++i)
        colors[i] = -1;
    IntArr y_arr = classifiers[x][y].predict(fValHist[t],actionHist[t],colors);
    stateHist[t].getColors(x, y, &colors);
    if (debug) cout << "Time " << t << " correct: " << printValarray(colors);
    if ((y_arr == colors).min() == true)
      correct++;
    else
      if (debug)
        cout << " Given: " << printValarray(y_arr);
    if (debug) cout << endl;
  }
  return correct / (float) fValHist.size();
};

/* Prints information about the activation pattern of a given feature. */
void Logistic::printFeatureInfo(int feat_num) {
  assert(feat_num >= 0 && feat_num < f.size());
  cout << f[feat_num].toString(feat_num) << endl;
  vector<int> active;
  vector<int> inactive;
  for (int t=0; t<fValHist.size(); ++t) {
    if (fValHist[t].size() >= feat_num) {
      if (fValHist[t][feat_num])
        active.push_back(t);
      else
        inactive.push_back(t);
    }
  }
  cout << "Timesteps active: ";
  for (int i=0; i<active.size(); ++i)
    cout << active[i] << " ";
  cout << endl;

  cout << "Timesteps inactive: ";
  for (int i=0; i<inactive.size(); ++i)
    cout << inactive[i] << " ";
  cout << endl;
};


/* Parses weights directly from a model file generated from crfsuite.
   c designates which classifier should recieve these weights.  */
void Logistic::parseWeightsFromFile(string filename, RegClassifier* c) {
  IntArr colors(0,numColors);
  if (!fexists(filename.c_str())) {
    perror("Cannot parse weights from non-existant file.\n");
    exit(-1);
  }
  string line;
  ifstream myfile (filename.c_str());
  int state = 0;
  if (myfile.is_open()) {
    while (myfile.good()) {
      getline(myfile,line);
      if (line.compare("TRANSITIONS = {") == 0) {
        state = 1;
        continue;
      } else if (line.compare("STATE_FEATURES = {") == 0) {
        state = 2;
        continue;
      } else if (line.compare("}") == 0) {
        state = 0;
        continue;
      }

      if (state == 1) { // Parsing state->state transition weights
        vector<string> v = split(line);
        assert(v.size() == 5);
        float feat_weight = atof(v[4].c_str());
        for (int i=0; i<8; ++i)
          colors[i] = atoi(v[1].substr(2*i+1,1).c_str());
        int from = c->getOutputIndx(colors);
        for (int i=0; i<8; ++i)
          colors[i] = atoi(v[3].substr(2*i+1,1).c_str());
        int to = c->getOutputIndx(colors);
        c->lambda_trans[from][to] = feat_weight;
      } else if (state == 2) { // Parsing features weights
        vector<string> v = split(line);
        assert(v.size() == 5);
        bool isAction = v[1][0] == 'a'; // Action or feature?
        int start_brkt = v[1].find("[");
        int end_brkt = v[1].find("]");
        int feat_num = atoi(v[1].substr(v[1].find("[")+1,end_brkt-start_brkt-1).c_str());
        bool feat_val = (bool) atoi(v[1].substr(v[1].length()-1).c_str());
        for (int i=0; i<8; ++i)
          colors[i] = atoi(v[3].substr(2*i+1,1).c_str());
        int output_val_indx = c->getOutputIndx(colors);
        float feat_weight = atof(v[4].c_str());
        if (feat_val) {
          if (isAction)
            c->action_on[output_val_indx][feat_num] = feat_weight;
          else
            c->lambda_on[output_val_indx][feat_num] = feat_weight;
        } else {
          if (isAction)
            c->action_off[output_val_indx][feat_num] = feat_weight;
          else
            c->lambda_off[output_val_indx][feat_num] = feat_weight;
        }
      }
    }
    myfile.close();
  }
  else
    cout << "Unable to open file";
};


/* Interfaces with the crfsuite library to optimize a given classifier. */
void Logistic::crfsuiteOptimize(int x, int y) {
  IntArr colors(0,numColors);
  CRFSuite::StringList yseq;
  CRFSuite::ItemSequence xseq;
  std::stringstream ss;

  // Model file is used to store the binary model
  ss << "crfsuitemdl" << x << "-" << y << ".out";
  string tmp_file(ss.str());
  ss.str(std::string()); // clear ss

  // Dump file is used to store the parsable dumped model
  ss << "dump" << x << "-" << y << ".out";
  string dump_file(ss.str());
  ss.str(std::string()); // clear ss

  // Translate our features into crfsuite features
  for (int t=0; t<fValHist.size(); ++t) {
    stateHist[t].getColors(x,y,&colors);
    yseq.push_back(printValarray(colors));
    CRFSuite::Item item;
    for (int k=0; k<fValHist[t].size(); ++k) {
      ss.str(std::string()); // clear ss
      ss << "f[" << k << "]=" << fValHist[t][k];
      CRFSuite::Attribute attr(ss.str());
      item.push_back(attr);
    }
    // Push back action
    for (int a=0; a<numPossibleActions; ++a) {
      ss.str(std::string()); // clear ss
      if (actionHist[t] == pv_possible_actions[a])
        ss << "a[" << a << "]=" << true;
      else
        ss << "a[" << a << "]=" << false;
      CRFSuite::Attribute attr(ss.str());
      item.push_back(attr);
    }
    
    xseq.push_back(item);
  }

  // Train the model
  CRFSuite::Trainer trainer;
  // if (debug)
  //LoggingTrainer trainer;
  trainer.append(xseq,yseq,0);
  trainer.select("lbfgs","crf1d");
  int ret = trainer.train(tmp_file.c_str(),-1);
  if (ret != 0) { cerr << "CRFSuite Training for (" << x << "," << y << ") failed with code " << ret << endl; }
  if (!fexists(tmp_file.c_str())) {
    cerr << "Error: CRFSuite Model file not found. Exiting." << endl;
    exit(-1);
  }

  // Dump the model
  crfsuite_model_t *model;
  ret = crfsuite_create_instance_from_file(tmp_file.c_str(), (void**)&model);
  FILE *fp;
  fp = fopen(dump_file.c_str(),"w");
  model->dump(model,fp);
  fclose(fp);

  // Parse the weights from the model dump file
  parseWeightsFromFile(dump_file, &classifiers[x][y]);

  if (remove(tmp_file.c_str()) != 0) cerr << "Unable to remove tmp model file." << endl;
  if (remove(dump_file.c_str()) != 0) cerr << "Unable to remove dump file." << endl;
};


void Logistic::exportARFF(int x, int y, string filename) {
  IntArr colors(0,numColors);
  ofstream f;
  f.open(filename.c_str());
  // Make sure we have the same number of data items
  for (int t=0; t<fValHist.size(); ++t)
    assert(fValHist[t].size() == fValHist[0].size());

  f << "@relation \'atari\'\n";
  for (int k=0; k<fValHist[0].size(); ++k)
    f << "@attribute f" << k << " {t,f}\n";

  f << "@attribute action {";
  for (int a=0; a < pv_possible_actions.size(); ++a) {
    f << pv_possible_actions[a];
    if (a != pv_possible_actions.size()-1)
      f << ",";
  }
  f << "}\n";

  f << "@attribute actionOLD {";
  for (int a=0; a < pv_possible_actions.size(); ++a) {
    f << pv_possible_actions[a];
    if (a != pv_possible_actions.size()-1)
      f << ",";
  }
  f << "}\n";

  f << "@attribute class {";
  for (int i=0; i<classifiers[x][y].outputVals.size(); ++i) {
    cout << i << " : " << printValarray(classifiers[x][y].outputVals[i]) << endl;
    f << i;
    if (i != classifiers[x][y].outputVals.size() -1)
      f << ",";
  }
  f << "}\n";

  f << "\n@data\n";

  for (int t=0; t<fValHist.size(); ++t) {
    for (int k=0; k<fValHist[t].size(); ++k) {
      if (fValHist[t][k])
        f << "t,";
      else
        f << "f,";
    }
    f << actionHist[t] << ",";
    if (t == 0)
      f << "0,";
    else
      f << actionHist[t-1] << ",";
    stateHist[t].getColors(x,y,&colors);
    f << classifiers[x][y].getOutputIndx(colors) << "\n";
  }

  f.close();
};

void Logistic::exportC45(int x, int y) {
  // Make sure we have the same number of data items
  // for (int t=0; t<fValHist.size(); ++t)
    //    assert(fValHist[t].size() == fValHist[0].size());

  IntArr colors(0,numColors);

  std::stringstream ss;
  ss << "c45data" << x << "-" << y;

  ofstream f;
  string namefile = ss.str() + ".names";
  f.open(namefile.c_str());

  for (int i=0; i<classifiers[x][y].outputVals.size(); ++i) {
    f << i;
    if (i != classifiers[x][y].outputVals.size() -1)
      f << ",";
  }
  f << ".\n\n";

  for (int k=0; k<fValHist.back().size(); ++k)
    f << "f" << k << ": true, false.\n";

  f << "action: ";
  for (int a=0; a < pv_possible_actions.size(); ++a) {
    f << pv_possible_actions[a];
    if (a != pv_possible_actions.size()-1)
      f << ",";
  }
  f << ".\n";

  f << "actionOLD: ";
  for (int a=0; a < pv_possible_actions.size(); ++a) {
    f << pv_possible_actions[a];
    if (a != pv_possible_actions.size()-1)
      f << ",";
  }
  f << ".\n";
  f.close();

  string datafile = ss.str() + ".data";
  f.open(datafile.c_str());
  for (int t=0; t<fValHist.size(); ++t) {
    for (int k=0; k<fValHist.back().size(); ++k) {
      if (fValHist[t].size() > k) {
        if (fValHist[t][k])
          f << "true,";
        else
          f << "false,";
      } else
        f << "?,";
    }
    f << actionHist[t] << ",";
    if (t == 0)
      f << "0,";
    else
      f << actionHist[t-1] << ",";
    stateHist[t].getColors(x,y,&colors);
    f << classifiers[x][y].getOutputIndx(colors) << "\n";
  }

  f.close();
};

void Logistic::createDecisionTree(int x, int y) {
  if (!system(NULL)) {
    printf("Processor Unavailable!\n");
    exit (-1);
  }

  std::stringstream ss;
  ss << x << "-" << y;
  string datafile = "c45data" + ss.str();
  string outputfile = "c45mdl" + ss.str();
  
  string command = "./R8/Src/c4.5 -f " + datafile + " > " + outputfile;

  if (system(command.c_str()) != 0) {
    printf("Error running the c4.5 program.\n");
    exit(-1);
  }

  ifstream f(outputfile.c_str());
  string line;
  map<string,treenode*> subtrees;
  assert(f.is_open());
  bool seen_simp_tree = false;
  while (f.good()) {
    getline(f,line);      
    if (line.compare("Decision Tree:")==0) {
      getline(f,line);      
      if (line.size() > 0) { // trivial decision tree
        vector<string> s = split(line);
        forest[x][y]->terminal = true;
        istringstream(s[0]) >> forest[x][y]->output;
      } else {
        parseDecisionTree(&f, forest[x][y], &subtrees);
      }
    } else if (line.compare("Simplified Decision Tree:")==0) {
      seen_simp_tree = true;
    } else if (line.find("Subtree") != string::npos && !seen_simp_tree) {
      int start = line.find('[');
      int end = line.find(']');
      string subtree_identifier = line.substr(start+1, end-start-1);
      getline(f,line);
      parseDecisionTree(&f, subtrees[subtree_identifier], &subtrees);
    }
  }
  f.close();

  if (remove((datafile+".names").c_str()) != 0) cerr << "Unable to remove c4.5 names file." << endl;
  if (remove((datafile+".data").c_str()) != 0) cerr << "Unable to remove c4.5 data file." << endl;
  if (remove((datafile+".tree").c_str()) != 0) cerr << "Unable to remove c4.5 tree file." << endl;
  if (remove((datafile+".unpruned").c_str()) != 0) cerr << "Unable to remove c4.5 unpruned file." << endl;
  if (remove(outputfile.c_str()) != 0) cerr << "Unable to remove c4.5 output file." << endl;
};

void Logistic::parseDecisionTree(ifstream* f, treenode *t, map<string,treenode*>* subtrees) {
  string line;
  getline(*f,line);
  treenode *curr_node = t;
  int indentation = 0;
  while (line.compare("") != 0) {
    vector<string> s = split(line);

    int new_indentation = 0;
    for (int i=0; i<s.size(); ++i) {
      if (s[i][0] != '|') {
        new_indentation = i;
        break;
      }
    }
	      
    while (new_indentation < indentation) {
      curr_node = curr_node->parent;
      indentation--;
    }

    assert(indentation == new_indentation);

    assert(curr_node->attribute.size() == 0 || curr_node->attribute.compare(s[new_indentation]) == 0);
    curr_node->attribute=s[new_indentation];
    curr_node->terminal = false;

    if (s.size() > new_indentation + 3) { // Terminal Node
      treenode *leaf= new treenode(curr_node);
      leaf->parent = curr_node;
      leaf->terminal = true;
      istringstream(s[new_indentation+3]) >> leaf->output;
      curr_node->children.push_back(leaf);
      curr_node->values.push_back(s[new_indentation+2]);
    } else { // Nonterminal node
      string value = s[new_indentation+2];
      treenode* child = new treenode(curr_node);
      child->parent = curr_node;
      curr_node->values.push_back(value);
      curr_node->children.push_back(child);
      curr_node = child;
      indentation++;
      // Check for a possible subtree
      if (value.find('[') != string::npos && value.find(']') != string::npos) {
        int start = value.find('[');
        int end = value.find(']');
        string subtree_identifier = value.substr(start+1, end-start-1);
        (*subtrees)[subtree_identifier] = curr_node;
        string corrected_value = value.substr(0,start);
        curr_node->parent->values.pop_back();
        curr_node->parent->values.push_back(corrected_value);        
      }
    }
    getline(*f,line);
  }
};

void Logistic::evaluateDecisionTree(int x, int y) {
  int correct = 0;
  int pred = forest[x][y]->predict(fValHist[0],actionHist[0],(Action)0);
  if (pred == classifiers[x][y].getOutputIndx(pv_tmp_color_bits))
    correct++;
  for (int t=1; t<fValHist.size(); ++t) {
    pred = forest[x][y]->predict(fValHist[t],actionHist[t],actionHist[t-1]);
    stateHist[t].getColors(x,y,&pv_tmp_color_bits);
    if (pred == classifiers[x][y].getOutputIndx(pv_tmp_color_bits))
      correct++;
    else
      cout << "Wrong at time: " << t << endl;
  }
  printf("Evaluation finished. %d/%d.\n", correct, (int)fValHist.size());
};

void Logistic::printDecisionTree(treenode* t, int depth) {

  for (int i=0; i<t->children.size(); ++i) {
    for (int j=0; j<depth; ++j)
      printf("|   ");
    cout << t->attribute << " = " << t->values[i];
    if (t->children[i]->terminal) cout << " " << t->children[i]->output;
    cout << endl;
    printDecisionTree(t->children[i],depth+1);
  }
};
