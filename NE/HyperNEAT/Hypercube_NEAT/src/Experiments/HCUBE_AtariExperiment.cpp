#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/foreach.hpp>
#include "../../../ale_v0.1/test.hpp"

using namespace NEAT;

enum GamePositionValue {
  EMPTY,
  CHICKEN,
  VEHICLE
};

namespace HCUBE
{
  AtariExperiment::AtariExperiment(string _experimentName,int _threadID):
    Experiment(_experimentName,_threadID), rom_file("../ale_v0.1/roms/asterix.bin"),
    display_active(false)
  {
    substrate_width = 16;
    substrate_height = 21;

    layerInfo = NEAT::LayeredSubstrateInfo();
    layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
    layerInfo.layerIsInput.push_back(true);
    layerInfo.layerLocations.push_back(Vector3<float>(0,0,0));
    layerInfo.layerNames.push_back("Input");

    layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
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

    //individual->reward(100);
    runAtariEpisode(individual);
  }

  void AtariExperiment::runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual) {
    // Initialize Atari Stuff 
    //initializeEmulator("../ale_v0.1/roms/asterix.bin",false);
    initializeEmulator(rom_file.c_str(),display_active);
    MediaSource &mediasrc = theOSystem->console().mediaSource();
    int pixel_screen_width = mediasrc.width();
    int pixel_screen_height = mediasrc.height();
    for (int i=0; i<pixel_screen_height; ++i) { // Initialize our screen matrix
      IntVect row;
      for (int j=0; j<pixel_screen_width; ++j)
        row.push_back(-1);
      screen_matrix.push_back(row);
    }
    // Intialize our ram array
    for (int i=0; i<RAM_LENGTH; i++)
      ram_content.push_back(0);

    System* emulator_system = &theOSystem->console().system();
    controller = (InternalController*) theOSystem->getGameController();
    self_detection_agent = (SelfDetectionAgent*) controller->getPlayerAgentLeft();
    game_settings = controller->getGameSettings();
    // Initialize Atari Stuff - fin

    // Main Loop
    int skip_frames_num = game_settings->i_skip_frames_num;
    int frame_skip_ctr = 0;

    // Reset the game mutliple times
    for (int i=0; i<5; i++)
      theOSystem->applyAction(RESET); 
    int delay_after_restart = game_settings->i_delay_after_restart;
    Action action = game_settings->e_first_action;
    if (action == UNDEFINED) action = PLAYER_A_NOOP;
    for (int i=0; i<delay_after_restart; i++)
      theOSystem->applyAction(action);
    time_start = time(NULL);
    bool game_ended = false;
    int frame = 0;
    float episode_reward = 0;
    while (!game_ended) {
      frame++;

      // Should we take an action this turn?
      bool take_action = frame_skip_ctr++ >= skip_frames_num;
      if (take_action) {
        frame_skip_ctr = 0;
        
        // Get the latest screen
        int ind_i, ind_j;
        uInt8* pi_curr_frame_buffer = mediasrc.currentFrameBuffer();
        for (int i = 0; i < pixel_screen_width * pixel_screen_height; i++) {
          uInt8 v = pi_curr_frame_buffer[i];
          ind_i = i / pixel_screen_width;
          ind_j = i - (ind_i * pixel_screen_width);
          screen_matrix[ind_i][ind_j] = v;
        }

        // Get the latest ram content
        for(int i = 0; i<RAM_LENGTH; i++) {
          int offset = i;
          offset &= 0x7f; // there are only 128 bytes
          ram_content[i] = emulator_system->peek(offset + 0x80);
        }

        float reward = game_settings->get_reward(&screen_matrix,&ram_content);
        episode_reward += reward;

        // Check if game has ended
        game_ended = game_settings->is_end_of_game(&screen_matrix,&ram_content,frame);

        // Get the object representation
        self_detection_agent->process_image(&screen_matrix, action);

        substrate.getNetwork()->reinitialize(); // Set value of all nodes to zero
        substrate.getNetwork()->dummyActivation();

        // Set substrate value for all objects (of a certain size)
        for (int i=0; i<self_detection_agent->obj_classes.size(); i++) {
          Prototype& proto = self_detection_agent->obj_classes[i];

          if (!proto.is_valid) // not a strong enough prototype yet
            continue;
          if (proto.obj_ids.size() == 0) // no obhects on screen
            continue;

          // Map object classes to values
          float assigned_value = proto.value;

          // Assign values to each of the objects
          for (set<long>::iterator it=proto.obj_ids.begin(); it!=proto.obj_ids.end(); it++) {
            long obj_id = *it;
            assert(self_detection_agent->composite_objs.find(obj_id) != self_detection_agent->composite_objs.end());
            point obj_centroid = self_detection_agent->composite_objs[obj_id].get_centroid();
            int adj_x = obj_centroid.x * substrate_width / pixel_screen_width;
            int adj_y = obj_centroid.y * substrate_height / pixel_screen_height;
            substrate.setValue((Node(adj_x,adj_y,0)),assigned_value);
          }
        }

        // Set substrate value for self
        point self_centroid = self_detection_agent->get_self_centroid();
        int self_x = -1, self_y = -1;
        if (self_centroid.x >= 0 && self_centroid.y >= 0) {
          self_x = self_centroid.x * substrate_width / pixel_screen_width;
          self_y = self_centroid.y * substrate_height / pixel_screen_height;
          substrate.setValue((Node(self_x,self_y,0)),1.0);
        }

        substrate.getNetwork()->update();

        // Choose which action to take
        if (self_x < 0 || self_y < 0) {
          action = PLAYER_A_NOOP;
        } else {
          float noop_val = substrate.getValue((Node(self_x,self_y,1)));
          float up_val   = (self_y <= 0) ? noop_val : substrate.getValue((Node(self_x,self_y-1,1)));
          float down_val = (self_y >= substrate_height-1) ? noop_val : substrate.getValue((Node(self_x,self_y+1,1)));
          float left_val = (self_x <= 0) ? noop_val : substrate.getValue((Node(self_x-1,self_y,1)));
          float right_val= (self_x >= substrate_width-1) ? noop_val : substrate.getValue((Node(self_x+1,self_y,1)));

          ActionVect *allowed_actions = game_settings->pv_possible_actions;
          Action actionIds[] = {PLAYER_A_NOOP,PLAYER_A_UP,PLAYER_A_DOWN,PLAYER_A_LEFT,PLAYER_A_RIGHT};
          float action_vals[] = {noop_val,up_val,down_val,left_val,right_val};
          
          int max_id = 0; // all games should have noop
          float max_val = action_vals[0];
          int size = sizeof(actionIds) / sizeof(Action);
          for (int i=1; i < size; i++) {
            if (action_vals[i] > max_val && 
                std::find(allowed_actions->begin(), allowed_actions->end(), actionIds[i]) != allowed_actions->end()) {
              max_val = action_vals[i];
              max_id = i;
            }
          }

          action = actionIds[max_id];
        }

        // Display the screen
        if (display_active)
          display_screen(screen_matrix);
      }

      // Apply action to simulator and update the simulator
      theOSystem->applyAction(action);
       
      if (frame % 1000 == 0) {
        time_end = time(NULL);
        double avg = ((double)frame)/(time_end - time_start);
        cerr << "Average main loop iterations per sec = " << avg << endl;
      }
    }

    // Give the reward to the agent
    individual->reward(episode_reward);
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
