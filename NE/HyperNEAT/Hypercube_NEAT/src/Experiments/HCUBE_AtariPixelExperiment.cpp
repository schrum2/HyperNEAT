#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariPixelExperiment.h"
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    uInt32 AtariPixelExperiment::eightBitPallete [256] = {
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0, 
        0, 0, 1, 0, 2, 0, 3, 0, 
        4, 0, 5, 0, 6, 0, 7, 0
    };


    AtariPixelExperiment::AtariPixelExperiment(string _experimentName,int _threadID):
        AtariExperiment(_experimentName,_threadID)
    {}

    void AtariPixelExperiment::initializeALE(string rom_name) {
        AtariExperiment::initializeALE(rom_name);
        // Set the dimensions of our substrate to be that of the screen
        substrate_width = ale.screen_width / 10;
        substrate_height = ale.screen_height / 10;
    }

    void AtariPixelExperiment::initializeTopology() {
        // Clear old layerinfo if present
        layerInfo.layerNames.clear();
        layerInfo.layerSizes.clear();
        layerInfo.layerValidSizes.clear();
        layerInfo.layerAdjacencyList.clear();
        layerInfo.layerIsInput.clear();
        layerInfo.layerLocations.clear();

        // One input layer for each color
        for (int i=0; i<numColors; i++) {
            layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
            layerInfo.layerIsInput.push_back(true);
            layerInfo.layerLocations.push_back(Vector3<float>(4*i,0,0));
            layerInfo.layerNames.push_back("Input" + boost::lexical_cast<std::string>(i));
        }

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

        for (int i=0; i<numColors; ++i) {
            layerInfo.layerAdjacencyList.push_back(std::pair<string,string>(
                                                       "Input" + boost::lexical_cast<std::string>(i),
                                                       "Processing"));
        }
        layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing","Output"));

        layerInfo.normalize = true;
        layerInfo.useOldOutputNames = false;
        layerInfo.layerValidSizes = layerInfo.layerSizes;

        substrate.setLayerInfo(layerInfo);
        outputLayerIndx = numColors + 1;
    }

    NEAT::GeneticPopulation* AtariPixelExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;

        // Input Nodes
        genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false)); // TODO: Check if this helps or not
        genes.push_back(GeneticNodeGene("X1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y1","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("X2","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("Y2","NetworkSensor",0,false));

        // Output Nodes
        for (int i=0; i<numColors; ++i) {
            genes.push_back(GeneticNodeGene("Output_Input" + boost::lexical_cast<std::string>(i) +
                                            "_Processing",
                                            "NetworkOutputNode",1,false,
                                            ACTIVATION_FUNCTION_SIGMOID));
        }
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

    void AtariPixelExperiment::setSubstrateValues(NEAT::LayeredSubstrate<float>* substrate) {
        for (int y=0; y<ale.screen_height; y++) {
            for (int x=0; x<ale.screen_width; x++) {
                int substrate_y = min(int((y / float(ale.screen_height)) * substrate_width), substrate_height-1);
                int substrate_x = min(int((x / float(ale.screen_width)) * substrate_width), substrate_width-1);
                uInt32 eightBitVal = eightBitPallete[ale.screen_matrix[y][x]];
                assert(eightBitVal < numColors);
                assert(substrate_x < substrate_width);
                assert(substrate_y < substrate_height);
                substrate->setValue((Node(substrate_x, substrate_y, eightBitVal)), 1.0);
            }
        }
    }
}
