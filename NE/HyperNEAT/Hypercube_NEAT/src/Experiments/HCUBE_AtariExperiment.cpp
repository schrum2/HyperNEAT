/**
   This is HyperNEAT run with the object representation.
 **/
#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    AtariExperiment::AtariExperiment(string _experimentName,int _threadID):
        Experiment(_experimentName,_threadID), substrate_width(8), substrate_height(10), visProc(NULL),
        rom_file(""), numActions(0), numObjClasses(0), display_active(false), outputLayerIndx(-1), epsilon(0)
    {
        if (NEAT::Globals::getSingleton()->hasParameterValue("epsilon")) {
            epsilon = NEAT::Globals::getSingleton()->getParameterValue("epsilon");
        }
        cout << "Using epsilon: " << epsilon << endl;
    }

    void AtariExperiment::initializeExperiment(string rom_file) {
        initializeALE(rom_file, true);
        initializeTopology();
    }

    void AtariExperiment::initializeALE(string rom_file, bool processScreen) {
        this->rom_file = rom_file;

        // Check that rom exists and is readable
        ifstream file(rom_file.c_str());
        if (!file.good()) {
            cerr << "Unable to find or open rom file: \"" << rom_file << "\"" << endl;
            exit(-1);
        }

        // Initialize Atari Stuff
        if (!ale.loadROM(rom_file.c_str(), display_active, processScreen)) {
            cerr << "Ale had problem loading rom..." << endl;
            exit(-1);
        }
        numActions = ale.legal_actions.size();

        if (processScreen) {
            // Load the visual processing framework
            visProc = ale.visProc;
            numObjClasses = visProc->manual_obj_classes.size();
            if (numObjClasses <= 0) {
                cerr << "No object classes found. Make sure there is an images directory containg class images." << endl;
                exit(-1);
            }
        }
    }

    void AtariExperiment::initializeTopology() {
        // Clear old layerinfo if present
        layerInfo.layerNames.clear();
        layerInfo.layerSizes.clear();
        layerInfo.layerValidSizes.clear();
        layerInfo.layerAdjacencyList.clear();
        layerInfo.layerIsInput.clear();
        layerInfo.layerLocations.clear();

        // One input layer for each object class
        for (int i=0; i<numObjClasses; ++i) {
            layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
            layerInfo.layerIsInput.push_back(true);
            layerInfo.layerLocations.push_back(Vector3<float>(4*i,0,0));
            layerInfo.layerNames.push_back("Input" + boost::lexical_cast<std::string>(i));
        }

        // One input layer for the self object
        layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        layerInfo.layerIsInput.push_back(true);
        layerInfo.layerLocations.push_back(Vector3<float>(4*numObjClasses,0,0));
        layerInfo.layerNames.push_back("InputSelf");

        // Processing level -- takes input from all the previous
        layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,4,0));
        layerInfo.layerNames.push_back("Processing");

        // Output layer -- used for action selection
        layerInfo.layerSizes.push_back(Vector2<int>(numActions,1));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,8,0));
        layerInfo.layerNames.push_back("Output");

        for (int i=0; i<numObjClasses; ++i) {
            layerInfo.layerAdjacencyList.push_back(std::pair<string,string>(
                                                       "Input" + boost::lexical_cast<std::string>(i),
                                                       "Processing"));
        }
        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("InputSelf","Processing"));
        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing","Output"));

        layerInfo.normalize = true;
        layerInfo.useOldOutputNames = false;
        layerInfo.layerValidSizes = layerInfo.layerSizes;

        substrate.setLayerInfo(layerInfo);
        outputLayerIndx = numObjClasses + 2;
    }

    NEAT::GeneticPopulation* AtariExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;

        // Input Nodes
        genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false)); // TODO: Check if this helps or not
        genes.push_back(GeneticNodeGene("X1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("X2","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y2","NetworkSensor",0,false));

        // Output Nodes
        for (int i=0; i<numObjClasses; ++i) {
            genes.push_back(GeneticNodeGene("Output_Input" + boost::lexical_cast<std::string>(i) +
                                            "_Processing",
                                            "NetworkOutputNode",1,false,
                                            ACTIVATION_FUNCTION_SIGMOID));
        }
        genes.push_back(GeneticNodeGene("Output_InputSelf_Processing","NetworkOutputNode",1,false,
                                        ACTIVATION_FUNCTION_SIGMOID));
        genes.push_back(GeneticNodeGene("Output_Processing_Output","NetworkOutputNode",1,false,
                                        ACTIVATION_FUNCTION_SIGMOID));

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

    void AtariExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
    {
        shared_ptr<NEAT::GeneticIndividual> individual = group.front();
        individual->setFitness(0);
        clock_t start = clock();
        substrate.populateSubstrate(individual);
        clock_t end = clock();
        cout << "Populated Substrate Size (" << substrate_width << "x" << substrate_height <<") in "
             << float(end-start)/CLOCKS_PER_SEC << " seconds." << endl;
        float score = runAtariEpisode(&substrate);
        individual->reward(score);
    }

    float AtariExperiment::runAtariEpisode(NEAT::LayeredSubstrate<float>* substrate) {
        ale.reset_game();
        
        while (!ale.game_over()) {
            // Set value of all nodes to zero
            substrate->getNetwork()->reinitialize(); 
            substrate->getNetwork()->dummyActivation();

            setSubstrateValues(substrate);

            // Propagate values through the ANN
            substrate->getNetwork()->update();

            // Print the Activations of the different layers
            //printLayerInfo(substrate);

            // Choose which action to take
            if (NEAT::Globals::getSingleton()->getRandom().getRandomDouble() < epsilon) {
                ale.act(selectRandomAction());
            } else {
                Action action = selectAction(substrate, outputLayerIndx);
                ale.act(action);
            }
        }
        cout << "Game ended in " << ale.frame << " frames with score " << ale.game_score << endl;
 
        return ale.game_score;
    }

    void AtariExperiment::setSubstrateValues(NEAT::LayeredSubstrate<float>* substrate) {
        // Set substrate value for all objects (of a certain size)
        setSubstrateObjectValues(*visProc, substrate);

        // Set substrate value for self
        setSubstrateSelfValue(*visProc, substrate);
    }

    void AtariExperiment::setSubstrateObjectValues(VisualProcessor& visProc,
                                                   NEAT::LayeredSubstrate<float>* substrate) {
        for (int i=0; i<visProc.manual_obj_classes.size(); i++) {
            Prototype& proto = visProc.manual_obj_classes[i];
            paintSubstrate(visProc, proto, substrate, i);
        }
    }

    void AtariExperiment::paintSubstrate(VisualProcessor& visProc, Prototype& proto,
                                         NEAT::LayeredSubstrate<float>* substrate, int substrateIndx) {
        // Assign values to each of the objects
        float assigned_value = 1.0;
        for (set<long>::iterator it=proto.obj_ids.begin(); it!=proto.obj_ids.end(); it++) {
            long obj_id = *it;
            assert(visProc.composite_objs.find(obj_id) != visProc.composite_objs.end());
            point obj_centroid = visProc.composite_objs[obj_id].get_centroid();
            int adj_x = obj_centroid.x * substrate_width / visProc.screen_width;
            int adj_y = obj_centroid.y * substrate_height / visProc.screen_height;
            // for (int y=0; y<substrate_height; ++y) {
            //     for (int x=0; x<substrate_width; ++x) {
            //         double val = gauss2D((double)x,(double)y, assigned_value,
            //                              (double)adj_x,(double)adj_y,1.0,1.0);
            //         substrate->setValue(Node(x,y,i),substrate->getValue(Node(x,y,i))+val);
            //     }
            // }
            substrate->setValue((Node(adj_x,adj_y,substrateIndx)),assigned_value);
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
		    // Schrum: Added color coding for substrates (Linux ONLY!).
		    // colors for different intensity and sign
		    if(val < -0.75) printf("\e[1;33m"); // yellow
		    else if(val < -0.5) printf("\e[1;31m"); // red
		    else if(val < -0.01) printf("\e[1;35m"); // magenta
		    else if(val > 0.75) printf("\e[1;36m"); // cyan
		    else if(val > 0.5) printf("\e[1;32m"); // green
		    else if(val > 0.01) printf("\e[1;34m"); // blue
		    else printf("\e[1;30m"); // gray
                    printf("%+1.1f ",val);
                    printf("\e[0m"); // rest to normal colors
                    // if (val >= .5)
                    //     printf("O");
                    // else
                    //     printf(" ");
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
        paintSubstrate(visProc, visProc.manual_self, substrate, numObjClasses);
    }

    Action AtariExperiment::selectAction(NEAT::LayeredSubstrate<float>* substrate, int outputLayerIndx) {
        vector<int> max_inds;
        float max_val = -1e37;
        for (int i=0; i < numActions; i++) {
            float output = substrate->getValue(Node(i,0,outputLayerIndx));
            if (output == max_val)
                max_inds.push_back(i);
            else if (output > max_val) {
                max_inds.clear();
                max_inds.push_back(i);
                max_val = output;
            }
        }
        int action_indx = NEAT::Globals::getSingleton()->getRandom().getRandomInt(max_inds.size());
        return ale.legal_actions[max_inds[action_indx]];
    }

    Action AtariExperiment::selectRandomAction() {
        int action_indx = NEAT::Globals::getSingleton()->getRandom().getRandomInt(ale.legal_actions.size());
        return ale.legal_actions[action_indx];
    }

    double AtariExperiment::gauss2D(double x, double y, double A, double mu_x, double mu_y,
                                    double sigma_x, double sigma_y)
    {
        return A * exp(-1.0 * ((x-mu_x) * (x-mu_x) / 2.0 * sigma_x * sigma_x + (y-mu_y) * (y-mu_y) / 2.0 * sigma_y * sigma_y));
    }
}
