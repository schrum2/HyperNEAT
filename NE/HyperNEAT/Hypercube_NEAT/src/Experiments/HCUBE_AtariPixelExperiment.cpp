/**
   This is HyperNEAT run with the raw pixel state representation.
 **/
#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariPixelExperiment.h"
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    // Each of the 256 possible colors are reduced to one of 8 colors.
    // This array defines the mapping.
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

    void AtariPixelExperiment::initializeExperiment(string rom_file) {
        initializeALE(rom_file, false); // No screen processing necessary

        // Set the dimensions of our substrate to be that of the screen
        substrate_width = ale.screen_width / 10;
        substrate_height = ale.screen_height / 10;

        initializeTopology();
    }

    // Schrum: Added to allow for more generality
    void AtariPixelExperiment::setProcessingLayers(int num) {
	numProcessingLayers = num;
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

	// cout << "Mine: Experiment class, processing layers: " << numProcessingLayers << endl;
        // Processing levels
	// Schrum: I enabled more than one processing layer
        for (int i=0; i<numProcessingLayers; i++) {
            layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
            layerInfo.layerIsInput.push_back(false);
            layerInfo.layerLocations.push_back(Vector3<float>(4*i,4,0));
            layerInfo.layerNames.push_back("Processing" + boost::lexical_cast<std::string>(i));
	}

        // Output layer
        layerInfo.layerSizes.push_back(Vector2<int>(numActions,1));
        layerInfo.layerIsInput.push_back(false);
        layerInfo.layerLocations.push_back(Vector3<float>(0,8,0));
        layerInfo.layerNames.push_back("Output");

        for (int i=0; i<numColors; ++i) {
	    // Schrum: connect to all processing layers
	    for (int j=0; j<numProcessingLayers; j++) {
            	layerInfo.layerAdjacencyList.push_back(std::pair<string,string>(
                                                       "Input" + boost::lexical_cast<std::string>(i),
                                                       "Processing" + boost::lexical_cast<std::string>(j)) );
	    }
        }

	// Schrum: All processing layers connect to output
	for (int i=0; i<numProcessingLayers; i++) {
            layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing" + boost::lexical_cast<std::string>(i),"Output"));
	}

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
	    // Schrum: output for each pairing of input and processing layers
	    for (int j=0; j<numProcessingLayers; j++) {
            	genes.push_back(GeneticNodeGene("Output_Input" + boost::lexical_cast<std::string>(i) +
                                                "_Processing"  + boost::lexical_cast<std::string>(j),
                                                "NetworkOutputNode",1,false,
                                                ACTIVATION_FUNCTION_SIGMOID));
	    }
        }

	// Schrum: link each processing layer to the output layer
	for (int j=0; j<numProcessingLayers; j++) {
            genes.push_back(GeneticNodeGene("Output_Processing" + boost::lexical_cast<std::string>(j) + "_Output","NetworkOutputNode",1,false,
                                            ACTIVATION_FUNCTION_SIGMOID));
	}

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
