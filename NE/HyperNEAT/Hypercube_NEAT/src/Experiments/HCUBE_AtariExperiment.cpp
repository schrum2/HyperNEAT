#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/foreach.hpp>
#include "common/visual_processor.h"
#include "common/random_tools.h"

using namespace NEAT;

namespace HCUBE
{
    AtariExperiment::AtariExperiment(string _experimentName,int _threadID):
        Experiment(_experimentName,_threadID), rom_file(""),
        display_active(false), currentSubstrateIndex(0)
    {

        // layerInfo = NEAT::LayeredSubstrateInfo();
        // layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        // layerInfo.layerIsInput.push_back(true);
        // layerInfo.layerLocations.push_back(Vector3<float>(0,0,0));
        // layerInfo.layerNames.push_back("Input");

        // layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        // layerInfo.layerIsInput.push_back(false);
        // layerInfo.layerLocations.push_back(Vector3<float>(0,4,0));
        // layerInfo.layerNames.push_back("Output");

        // layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Input","Output"));

        // layerInfo.normalize = true;
        // layerInfo.useOldOutputNames = false;
        // layerInfo.layerValidSizes = layerInfo.layerSizes;

        // substrate = NEAT::LayeredSubstrate<float>();
        // substrate.setLayerInfo(layerInfo);

        // This can be re-initialized if necessary
        initializeExperiment("/home/matthew/projects/HyperNEAT/ale_v0.1/roms/asterix.bin");
    }

    void AtariExperiment::initializeExperiment(string _rom_file) {
        rom_file = _rom_file;

        substrate_width = 16;
        substrate_height = 21;

        // Check that rom exists and is readable
        ifstream file(rom_file.c_str());
        if (!file.good()) {
            cerr << "Unable to find or open rom file: \"" << rom_file << "\"" << endl;
            exit(-1);
        }

        // Initialize Atari Stuff
        if (!ale.loadROM(rom_file.c_str(), display_active)) {
            cerr << "Ale had problem loading rom..." << endl;
            exit(-1);
        }
        numActions = ale.allowed_actions->size();

        // Clear old layerinfo if present
        layerInfo.layerNames.clear();
        layerInfo.layerSizes.clear();
        layerInfo.layerValidSizes.clear();
        layerInfo.layerAdjacencyList.clear();
        layerInfo.layerIsInput.clear();
        layerInfo.layerLocations.clear();

        layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        layerInfo.layerIsInput.push_back(true);
        layerInfo.layerLocations.push_back(Vector3<float>(0,0,0));
        layerInfo.layerNames.push_back("Input");

        layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,4,0));
        layerInfo.layerNames.push_back("Processing");

        layerInfo.layerSizes.push_back(Vector2<int>(numActions,1));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,8,0));
        layerInfo.layerNames.push_back("Output");

        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Input","Processing"));
        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing","Output"));

        layerInfo.normalize = true;
        layerInfo.useOldOutputNames = true;
        layerInfo.layerValidSizes = layerInfo.layerSizes;

        for (int a=0; a<2; a++)
            substrates[a].setLayerInfo(layerInfo);
    }

    NEAT::GeneticPopulation* AtariExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;

        genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("X1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("X2","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y2","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Output_ab","NetworkOutputNode",1,false,
                                        ACTIVATION_FUNCTION_SIGMOID));
        genes.push_back(GeneticNodeGene("Output_bc","NetworkOutputNode",1,false,
                                        ACTIVATION_FUNCTION_SIGMOID));


        // genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false));
        // genes.push_back(GeneticNodeGene("X1","NetworkSensor",0,false));
        // genes.push_back(GeneticNodeGene("X2","NetworkSensor",0,false));
        // genes.push_back(GeneticNodeGene("Y1","NetworkSensor",0,false));
        // genes.push_back(GeneticNodeGene("Y2","NetworkSensor",0,false));
        // genes.push_back(GeneticNodeGene("Output_Input_Output","NetworkOutputNode",1,false,ACTIVATION_FUNCTION_SIGMOID));

        for (int a=0; a<populationSize; a++) {
            shared_ptr<GeneticIndividual> individual(new GeneticIndividual(genes,true,1.0));
            for (int b=0;b<10;b++) {
                individual->testMutate();
            }
            population->addIndividual(individual);
        }

        cout << "Finished creating population\n";
        return population;
    }

    void AtariExperiment::populateSubstrate(shared_ptr<NEAT::GeneticIndividual> individual,
        int substrateNum) {
        NEAT::LayeredSubstrate<float>* substrate;
        if (substrateNum>=2)
            throw CREATE_LOCATEDEXCEPTION_INFO("ERROR: INVALID SUBSTRATE INDEX!");

        //Don't bother remaking the same substrate
        if (substrateIndividuals[substrateNum]==individual)
            return;

        substrateIndividuals[substrateNum]=individual;

        substrate = &substrates[substrateNum];

        substrate->populateSubstrate(individual);
    }

    void AtariExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
    {
        shared_ptr<NEAT::GeneticIndividual> individual = group.front();
        individual->setFitness(10);

        // Print the individual
        //individual->print();

        populateSubstrate(individual);

        //individual->reward(100);
        runAtariEpisode(individual);
    }

    void AtariExperiment::runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual) {
        NEAT::LayeredSubstrate<float>* substrate = &substrates[currentSubstrateIndex];

        // Reset the game
        ale.reset_game();
        
        // Computes high level visual representation of the screen
        VisualProcessor visProc(ale.theOSystem, ale.game_settings);
        
        while (!ale.game_over()) {
            // Get the object representation
            visProc.process_image(&ale.screen_matrix, ale.last_action);

            substrate->getNetwork()->reinitialize(); // Set value of all nodes to zero
            substrate->getNetwork()->dummyActivation();

            // Set substrate value for all objects (of a certain size)
            setSubstrateObjectValues(visProc, substrate);

            // Set substrate value for self
            //setSubstrateSelfValue(visProc, substrate);

            // Propagate values through the ANN
            substrate->getNetwork()->update();

            // Print the Activations of the different layers
            //printLayerInfo(substrate);

            // Choose which action to take
            Action action = selectAction(visProc, substrate);
            ale.apply_action(action);
        }
 
        // Give the reward to the agent
        individual->reward(ale.game_score);
    }

    void AtariExperiment::setSubstrateObjectValues(VisualProcessor& visProc,
                                                   NEAT::LayeredSubstrate<float>* substrate) {
        for (int i=0; i<visProc.obj_classes.size(); i++) {
            Prototype& proto = visProc.obj_classes[i];

            if (!proto.is_valid) // not a strong enough prototype yet
                continue;
            if (proto.obj_ids.size() == 0) // no obhects on screen
                continue;

            // Map object classes to values
            float assigned_value = 0;
            // HACK: Hardcoded mapping from object classes to values
            //if (proto.size == 66 || proto.size == 70) // Freeway

            // Asterix
            if (proto.size == 41) assigned_value = 1;
            if (proto.size == 42) assigned_value = -1;

            // Assign values to each of the objects
            for (set<long>::iterator it=proto.obj_ids.begin(); it!=proto.obj_ids.end(); it++) {
                long obj_id = *it;
                assert(visProc->composite_objs.find(obj_id) != visProc->composite_objs.end());
                point obj_centroid = visProc.composite_objs[obj_id].get_centroid();
                int adj_x = obj_centroid.x * substrate_width / visProc.screen_width;
                int adj_y = obj_centroid.y * substrate_height / visProc.screen_height;
                for (int y=0; y<substrate_height; ++y) {
                    for (int x=0; x<substrate_width; ++x) {
                        double val = gauss2D((double)x,(double)y, assigned_value,
                                             (double)adj_x,(double)adj_y,1.0,1.0);
                        substrate->setValue(Node(x,y,0),substrate->getValue(Node(x,y,0))+val);
                    }
                }
                //substrate->setValue((Node(adj_x,adj_y,0)),assigned_value);
            }
        }
    }

    void AtariExperiment::printLayerInfo(NEAT::LayeredSubstrate<float>* substrate) {
        for (int i=0; i<layerInfo.layerNames.size(); i++) {
            string layerName = layerInfo.layerNames[i];
            JGTL::Vector2<int> layerSize = layerInfo.layerSizes[i];
            JGTL::Vector2<int> layerValidSize = layerInfo.layerValidSizes[i];
            bool isInput = layerInfo.layerIsInput[i];
            printf("Layer%d Name:%s Size:<%d,%d> ValidSize:<%d,%d> Input:%d\n",i,layerName.c_str(),
                   layerSize.x,layerSize.y,layerValidSize.x,layerValidSize.y,isInput);
                
            for (int y=0; y<layerSize.y; ++y) {
                for (int x=0; x<layerSize.x; ++x) {
                    float val = substrate->getValue(Node(x,y,i));
                    printf("%1.1f ",val);
                }
                printf("\n");
            }
            printf("\n");
        }
        cin.get();
    }


    void AtariExperiment::setSubstrateSelfValue(VisualProcessor& visProc,
                                                NEAT::LayeredSubstrate<float>* substrate) {
        if (!visProc.found_self())
            return;

        point self_centroid = visProc.get_self_centroid();
        int self_x = self_centroid.x * substrate_width / visProc.screen_width;
        int self_y = self_centroid.y * substrate_height / visProc.screen_height;
                    
        // for (int y=0; y<substrate_height; ++y) {
        //     for (int x=0; x<substrate_width; ++x) {
        //         double val = gauss2D((double)x,(double)y,1.0,(double)self_x,(double)self_y,.5,.5);
        //         substrate->setValue(Node(x,y,0),substrate->getValue(Node(x,y,0))+val);
        //     }
        // }
        //substrate->setValue(Node(self_x,self_y,0),1.0);
    }

    Action AtariExperiment::selectAction(VisualProcessor& visProc,
                                  NEAT::LayeredSubstrate<float>* substrate) {
        vector<int> max_inds;
        float max_val = -1e37;
        for (int i=0; i < numActions; i++) {
            float output = substrate->getValue(Node(i,0,2));
            if (output == max_val)
                max_inds.push_back(i);
            else if (output > max_val) {
                max_inds.clear();
                max_inds.push_back(i);
                max_val = output;
            }
        }
        int action_indx = choice(&max_inds);
        return (*ale.allowed_actions)[action_indx];


        // // If no self detected, take a random action.
        // if (!visProc.found_self()) {
        //     //printf("Unable to detect the self. Taking random action.\n");
        //     return (*ale.allowed_actions)[rand() % ale.allowed_actions->size()];
        // }

        // point self_centroid = visProc.get_self_centroid();
        // int self_x = self_centroid.x * substrate_width / visProc.screen_width;
        // int self_y = self_centroid.y * substrate_height / visProc.screen_height;

        // // Choose which action to take
        // float noop_val = substrate->getValue((Node(self_x,self_y,1)));
        // float up_val   = (self_y <= 0) ? noop_val : substrate->getValue((Node(self_x,self_y-1,1)));
        // float down_val = (self_y >= substrate_height-1) ? noop_val : substrate->getValue((Node(self_x,self_y+1,1)));
        // float left_val = -1e37;//(self_x <= 0) ? noop_val : substrate->getValue((Node(self_x-1,self_y,1)));
        // float right_val= -1e37;//(self_x >= substrate_width-1) ? noop_val : substrate->getValue((Node(self_x+1,self_y,1)));

        // Action actionIds[] = {PLAYER_A_NOOP,PLAYER_A_UP,PLAYER_A_DOWN,PLAYER_A_LEFT,
        //                       PLAYER_A_RIGHT};

        // float action_vals[] = {noop_val,up_val,down_val,left_val,right_val};
          
        // int max_id = 0; // all games should have noop
        // float max_val = action_vals[0];
        // int size = sizeof(actionIds) / sizeof(Action);
        // for (int i=1; i < size; i++) {
        //     if (action_vals[i] > max_val && 
        //         std::find(ale.allowed_actions->begin(), ale.allowed_actions->end(), actionIds[i]) != ale.allowed_actions->end()) {
        //         max_val = action_vals[i];
        //         max_id = i;
        //     }
        // }
        // return actionIds[max_id];
    }

    double AtariExperiment::gauss2D(double x, double y, double A, double mu_x, double mu_y,
                                    double sigma_x, double sigma_y)
    {
        return A * exp(-1.0 * ((x-mu_x) * (x-mu_x) / 2.0 * sigma_x * sigma_x + (y-mu_y) * (y-mu_y) / 2.0 * sigma_y * sigma_y));
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
