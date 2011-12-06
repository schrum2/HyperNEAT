#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/foreach.hpp>
#include "../../../ale_v0.1/test.hpp"

/*
int test(int argc, char* argv[]) {
  initializeEmulator();

  // Media assets
  MediaSource& mediasrc = theOSystem->console().mediaSource();
  int screen_width = mediasrc.width();
  int screen_height = mediasrc.height();
  IntMatrix pm_screen_matrix; // 2D Matrix containing screen pixel colors
  for (int i=0; i<screen_height; ++i) { // Initialize our matrix
    IntVect row;
    for (int j=0; j<screen_width; ++j)
      row.push_back(-1);
    pm_screen_matrix.push_back(row);
  }

  InternalController* controller = (InternalController*) theOSystem->getGameController();
  SelfDetectionAgent* self_detection_agent = (SelfDetectionAgent*) controller->getPlayerAgentLeft();
  GameSettings* game_settings = controller->getGameSettings();

  // Main Loop
  int skip_frames_num = game_settings->i_skip_frames_num;
  int frame_skip_ctr = 0;
  Action action = RESET;
  time_start = time(NULL);
  for (int frame=0; frame<100000; frame++) {
    if (frame_skip_ctr++ >= skip_frames_num) {
      frame_skip_ctr = 0;
      
      // Get the latest screen
      int ind_i, ind_j;
      uInt8* pi_curr_frame_buffer = mediasrc.currentFrameBuffer();
      for (int i = 0; i < screen_width * screen_height; i++) {
        uInt8 v = pi_curr_frame_buffer[i];
        ind_i = i / screen_width;
        ind_j = i - (ind_i * screen_width);
        pm_screen_matrix[ind_i][ind_j] = v;
      }

      // Get the object representation
      self_detection_agent->process_image(&pm_screen_matrix, action);

      // Choose Action
      if (frame <5) action = RESET;
      else action = PLAYER_A_UP;

      // Display the screen
      display_screen(pm_screen_matrix);
    }

    // Apply action to simulator and update the simulator
    theOSystem->applyAction(action);
    
    if (frame % 1000 == 0) {
      time_end = time(NULL);
      double avg = ((double)frame)/(time_end - time_start);
      cerr << "Average main loop iterations per sec = " << avg << endl;
    }
  }

  cleanup();
  return 0;
}
*/

using namespace NEAT;

enum GamePositionValue {
  EMPTY,
  CHICKEN,
  VEHICLE
};

namespace HCUBE
{
  AtariExperiment::AtariExperiment(string _experimentName,int _threadID):
    Experiment(_experimentName,_threadID)
  {
    screen_width = 16;
    screen_height = 19;

    layerInfo = NEAT::LayeredSubstrateInfo();
    layerInfo.layerSizes.push_back(Vector2<int>(screen_width,screen_height));
    layerInfo.layerIsInput.push_back(true);
    layerInfo.layerLocations.push_back(Vector3<float>(0,0,0));
    layerInfo.layerNames.push_back("Input");

    layerInfo.layerSizes.push_back(Vector2<int>(screen_width,screen_height));
    layerInfo.layerIsInput.push_back(false);
    layerInfo.layerLocations.push_back(Vector3<float>(0,4,0));
    layerInfo.layerNames.push_back("Output");

    layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Input","Output"));

    layerInfo.normalize = true;
    layerInfo.useOldOutputNames = false;
    layerInfo.layerValidSizes = layerInfo.layerSizes;

    substrate = NEAT::LayeredSubstrate<float>();
    substrate.setLayerInfo(layerInfo);

  }

  NEAT::GeneticPopulation* AtariExperiment::createInitialPopulation(int populationSize) {
    GeneticPopulation *population = new GeneticPopulation();
    vector<GeneticNodeGene> genes;

    genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false));
    genes.push_back(GeneticNodeGene("X1","NetworkSensor",0,false));
    genes.push_back(GeneticNodeGene("X2","NetworkSensor",0,false));
    genes.push_back(GeneticNodeGene("Y1","NetworkSensor",0,false));
    genes.push_back(GeneticNodeGene("Y2","NetworkSensor",0,false));
    genes.push_back(GeneticNodeGene("Output_Input_Output","NetworkOutputNode",1,false,ACTIVATION_FUNCTION_SIGMOID));

    for (int a=0;a<populationSize;a++) {
      shared_ptr<GeneticIndividual> individual(new GeneticIndividual(genes,true,1.0));
      for (int b=0;b<10;b++) {
        individual->testMutate();
      }
      population->addIndividual(individual);
    }

    cout << "Finished creating population\n";
    return population;
  }

  void AtariExperiment::populateSubstrate(shared_ptr<NEAT::GeneticIndividual> individual) {
    if (currentSubstrateIndividual == individual)
      return;

    currentSubstrateIndividual = individual;
    substrate.populateSubstrate(individual);
  }

  void AtariExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
  {
    static int i=0;
    
    shared_ptr<NEAT::GeneticIndividual> individual = group.front();
    individual->setFitness(10);

    // Print the individual
    //individual->print();

    populateSubstrate(individual);

    runAtariEpisode(individual);
  }

  void AtariExperiment::runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual) {
   
    // Initialize Atari Stuff 
    initializeEmulator();
    MediaSource &mediasrc = theOSystem->console().mediaSource();
    int pixel_screen_width = mediasrc.width();
    int pixel_screen_height = mediasrc.height();
    for (int i=0; i<pixel_screen_height; ++i) { // Initialize our matrix
      IntVect row;
      for (int j=0; j<pixel_screen_width; ++j)
        row.push_back(-1);
      pm_screen_matrix.push_back(row);
    }
  
    controller = (InternalController*) theOSystem->getGameController();
    self_detection_agent = (SelfDetectionAgent*) controller->getPlayerAgentLeft();
    game_settings = controller->getGameSettings();
    // Initialize Atari Stuff - fin

    // Main Loop
    int skip_frames_num = game_settings->i_skip_frames_num;
    int frame_skip_ctr = 0;
    Action action = RESET;
    time_start = time(NULL);
    for (int frame=0; frame<2000; frame++) {
      if (frame_skip_ctr++ >= skip_frames_num) {
        frame_skip_ctr = 0;
        
        // Get the latest screen
        int ind_i, ind_j;
        uInt8* pi_curr_frame_buffer = mediasrc.currentFrameBuffer();
        for (int i = 0; i < pixel_screen_width * pixel_screen_height; i++) {
          uInt8 v = pi_curr_frame_buffer[i];
          ind_i = i / pixel_screen_width;
          ind_j = i - (ind_i * pixel_screen_width);
          pm_screen_matrix[ind_i][ind_j] = v;
        }

        // Get the object representation
        self_detection_agent->process_image(&pm_screen_matrix, action);

        substrate.getNetwork()->reinitialize();
        substrate.getNetwork()->dummyActivation();

        // TODO Set substrate values
        for (int x=0; x<screen_width; ++x) {
          for (int y=0; y<screen_height; ++y) {
            substrate.setValue((Node(x,y,0)), 0.5 * (rand() % 3));
          }
        }

        substrate.getNetwork()->update();

        // TODO Choose action here NONTRIVIAL
        //float chicken_val = substrate.getValue((Node(chic_x,chic_y,1)));
        //float down_val = (chic_y >= screen_height-1) ? chicken_val : substrate.getValue((Node(chic_x,chic_y+1,1)));
        //float up_val = (chic_y <= 0) ? chicken_val : substrate.getValue((Node(chic_x,chic_y-1,1)));

        // TODO Choose Action - This should go away
        if (frame <5) action = RESET;
        else action = PLAYER_A_UP;

        // Display the screen
        // display_screen(pm_screen_matrix);
      }

      // Apply action to simulator and update the simulator
      theOSystem->applyAction(action);
       
      if (frame % 1000 == 0) {
        time_end = time(NULL);
        double avg = ((double)frame)/(time_end - time_start);
        cerr << "Average main loop iterations per sec = " << avg << endl;
      }
      
    }

    cout << "[Atari] Attempting to cleanup" << endl << flush;
    //cleanup();
    cout << "[Atari] Finished cleanup" << endl << flush;

    // TODO Give reward to the agent - change here
    individual->reward(rand() % 10);
    cout << "[Atari] Gave reward" << endl << flush;
  }

  void AtariExperiment::preprocessIndividual(shared_ptr<NEAT::GeneticGeneration> generation,
                                             shared_ptr<NEAT::GeneticIndividual> individual) {
    if (individual->getNode("X1") == NULL) {
      printf("Got blank individual\n");
    }
  }

  void AtariExperiment::processIndividualPostHoc(shared_ptr<NEAT::GeneticIndividual> individual)
  {
    // NEAT::FastNetwork<float> network = individual->spawnFastPhenotypeStack<float>();

    // //TODO Put in userdata

    // double fitness = 10.0;
    // double maxFitness = 10.0;

    // for (int x1=0;x1<2;x1++)
    //   {
    //     for (int x2=0;x2<2;x2++)
    //       {
    //         network.reinitialize();

    //         network.setValue("X1",x1);
    //         network.setValue("X2",x2);
    //         network.setValue("Bias",0.3f);

    //         network.update();

    //         double value = network.getValue("Output");

    //         double expectedValue = (double)(x1 ^ x2);

    //         fitness += (5000*(2-fabs(value-expectedValue)));
    //         maxFitness += 5000*2;
    //       }
    //   }

    // cout << "POST HOC ANALYSIS: " << fitness << "/" << maxFitness << endl;
  }

  Experiment* AtariExperiment::clone()
  {
    AtariExperiment* experiment = new AtariExperiment(*this);

    return experiment;
  }
}
