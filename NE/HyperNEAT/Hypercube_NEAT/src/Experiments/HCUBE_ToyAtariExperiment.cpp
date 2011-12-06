#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/foreach.hpp>

using namespace NEAT;

enum GamePositionValue {
  EMPTY,
  CHICKEN,
  VEHICLE
};

namespace HCUBE
{
  AtariExperiment::AtariExperiment(string _experimentName,int _threadID):
    Experiment(_experimentName,_threadID),
    screen_width(5), screen_height(19)
  {
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
    GamePositionValue gameState[screen_width][screen_height];

    int chic_x = screen_width/2, chic_y = screen_height-1;
    float total_reward = 0.0;
    
    // Initialize Game
    for (int x=0;x<screen_width;x++) {
      for (int y=0;y<screen_height;y++) {
        gameState[x][y] = EMPTY;
      }
    }

    // Initialize Chicken
    gameState[chic_x][chic_y] = CHICKEN;

    // Initialize Cars
    for (int y=2; y<=screen_height-2; y+=2) {
      if (y < screen_height/2)
        gameState[screen_width-1][y] = VEHICLE;
      else
        gameState[0][y] = VEHICLE;
    }

    // Run simulation for t timesteps
    int max_num_episodes = 100;
    int episode = 0;
    int curr_time = 0, max_timesteps = 100;
    while (episode < max_num_episodes) {
      curr_time++;
      substrate.getNetwork()->reinitialize();
      substrate.getNetwork()->dummyActivation();

      // Print Game Screen
      // if (threadID == 0) {
      //   printf("Timestep %d Individual Species ID %d\n",t,individual->getSpeciesID());
      //   for (int x=0; x<screen_width+2; ++x)
      //     printf("-");
      //   printf("\n");
      //   for (int y=0; y<screen_height; ++y) {
      //     for (int x=0; x<screen_width; ++x) {
      //       if (x==0) printf("|");
      //       if (gameState[x][y] == CHICKEN) {
      //         printf("x");
      //       } else if (gameState[x][y] == VEHICLE) {
      //         printf("o");
      //       } else {
      //         printf(" ");
      //       }
      //     }
      //     printf("|\n");
      //   }
      //   for (int x=0; x<screen_width+2; ++x)
      //     printf("-");
      //   printf("\n");
      //   cin.get();
      // }

      // Set substrate values
      for (int x=0; x<screen_width; ++x) {
        for (int y=0; y<screen_height; ++y) {
          if (gameState[x][y] == CHICKEN) {
            substrate.setValue((Node(x,y,0)), 1.0);
          } else if (gameState[x][y] == VEHICLE) {
            // Highlight regions that the car is going to move into
            // for (int x2=max(0,x-2); x2<min(screen_width,x+3); x2++) {
            //   if (gameState[x2][y] == CHICKEN) continue;
            //   substrate.setValue((Node(x2,y,0)), -.5);
            // }
            substrate.setValue((Node(x,y,0)), -1.0);
          } else {
            substrate.setValue((Node(x,y,0)), 0.0);            
          }
        }
      }

      substrate.getNetwork()->update();

      float chicken_val = substrate.getValue((Node(chic_x,chic_y,1)));
      float down_val = (chic_y >= screen_height-1) ? chicken_val : substrate.getValue((Node(chic_x,chic_y+1,1)));
      float up_val = (chic_y <= 0) ? chicken_val : substrate.getValue((Node(chic_x,chic_y-1,1)));

      int action;
      if (chicken_val >= up_val) {
        if (chicken_val >= down_val) {
          action = 0;
        } else {
          action = 1;
        }
      } else {
        if (up_val >= down_val) {
          action = -1;
        } else {
          action = 1;
        }
      }
      //action = -1;
      
      // Update game state with action
      bool collision = false;

      // Move cars
      for (int y=0; y<screen_height; ++y) {
        for (int x=0; x<screen_width; ++x) {
          if (gameState[x][y] == VEHICLE) {
            int velocity = rand()%2;
            int carx;
            if (y < screen_height/2) { // Top half cars go left
              if (x == 0) // reset car if it has reached the end
                carx = screen_width-1; 
              else
                carx = max(0, x-velocity);
            } else {
              if (x == screen_width-1)
                carx = 0;
              else
                carx = min(screen_width-1, x+velocity);
            }
            gameState[x][y] = EMPTY;
            gameState[carx][y] = VEHICLE;
            break;
          }
        }
      }

      // Move chicken
      if (gameState[chic_x][chic_y] != VEHICLE) gameState[chic_x][chic_y] = EMPTY;
      if (gameState[chic_x][chic_y+action] != VEHICLE) gameState[chic_x][chic_y+action] = CHICKEN;
      else collision = true;
      chic_y += action;

      // Compute reward
      if (chic_y == 0 || collision || curr_time >= max_timesteps) {
        episode++;
        
        if (chic_y == 0) {
          total_reward += 50;
        } else if (collision) {
          total_reward += screen_height - chic_y;
        }
        
        curr_time = 0;
        // Reset the sim
        if (gameState[chic_x][chic_y] != VEHICLE) gameState[chic_x][chic_y] = EMPTY;
        gameState[chic_x][screen_height-1] = CHICKEN;
        chic_y = screen_height-1;
      }
    }
    float avg_reward = total_reward / max_num_episodes;
    //cout << "Got total reward: " << total_reward << endl;
    individual->reward(avg_reward);
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
