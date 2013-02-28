#include "HCUBE_Defines.h"
#include "NEAT_Random.h"

#include "Experiments/HCUBE_AtariNoiseExperiment.h"

using namespace NEAT;

namespace HCUBE
{
    AtariNoiseExperiment::AtariNoiseExperiment(string _experimentName,int _threadID):
        AtariExperiment(_experimentName,_threadID)
    {}

    void AtariNoiseExperiment::initializeALE(string rom_name) {
        AtariExperiment::initializeALE(rom_name);
        // Set the dimensions of our substrate to be that of the screen
        substrate_width = ale.screen_width / 10;
        substrate_height = ale.screen_height / 10;
    }

    void AtariNoiseExperiment::initializeTopology() {
        // Clear old layerinfo if present
        layerInfo.layerNames.clear();
        layerInfo.layerSizes.clear();
        layerInfo.layerValidSizes.clear();
        layerInfo.layerAdjacencyList.clear();
        layerInfo.layerIsInput.clear();
        layerInfo.layerLocations.clear();

        // One input layer 
        layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        layerInfo.layerIsInput.push_back(true);
        layerInfo.layerLocations.push_back(Vector3<float>(0,0,0));
        layerInfo.layerNames.push_back("Input");

        // Processing level
        layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,4,0));
        layerInfo.layerNames.push_back("Processing");

        // Output layer
        layerInfo.layerSizes.push_back(Vector2<int>(numActions,1));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,8,0));
        layerInfo.layerNames.push_back("Output");

        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Input","Processing"));
        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing","Output"));

        layerInfo.normalize = true;
        layerInfo.useOldOutputNames = false;
        layerInfo.layerValidSizes = layerInfo.layerSizes;

        substrate.setLayerInfo(layerInfo);
        outputLayerIndx = 2;
    }

    NEAT::GeneticPopulation* AtariNoiseExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;

        // Input Nodes
        genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false)); // TODO: Check if this helps or not
        genes.push_back(GeneticNodeGene("X1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("X2","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y2","NetworkSensor",0,false));

        // Output Nodes
        genes.push_back(GeneticNodeGene("Output_Input_Processing", "NetworkOutputNode",1,false,
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

    void AtariNoiseExperiment::setSubstrateValues(NEAT::LayeredSubstrate<float>* substrate) {
        NEAT::Random& random = NEAT::Globals::getSingleton()->getRandom();
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                substrate->setValue((Node(x, y, 0)), random.getRandomDouble());
            }
        }
    }
}
