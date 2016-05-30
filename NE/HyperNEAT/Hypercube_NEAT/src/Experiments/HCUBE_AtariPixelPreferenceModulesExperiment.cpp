/**
   This is HyperNEAT run with the raw pixel state representation.
   Schrum: allows for multiple output modules with arbitration
           via preference neurons.
 **/
#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariPixelPreferenceModulesExperiment.h"
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    AtariPixelPreferenceModulesExperiment::AtariPixelPreferenceModulesExperiment(string _experimentName,int _threadID):
        AtariPixelExperiment(_experimentName,_threadID)
    {}

    // Schrum: Added to be able to set the number of modules
    void AtariPixelPreferenceModulesExperiment::setOutputModules(int num) {
	numOutputModules = num;
    }

    void AtariPixelPreferenceModulesExperiment::initializeTopology() {
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

        // Schrum: multiple processing levels
	for (int j=0; j<numProcessingLevels; j++) {
	    // Schrum: I enabled more than one processing substrate per layer
            for (int i=0; i<numProcessingLayers; i++) {
                layerInfo.layerSizes.push_back(Vector2<int>(substrate_width,substrate_height));
                layerInfo.layerIsInput.push_back(false);
                layerInfo.layerLocations.push_back(Vector3<float>(4*i,4+(4*j),0));
                layerInfo.layerNames.push_back("Processing" + boost::lexical_cast<std::string>(j) + "-" + boost::lexical_cast<std::string>(i));
	    }
	}

        // Output layer
        // Schrum: Allow multiple output substrates/modules
        // Every output module has a corresponding preference neuron in its own separate substrate
	for (int i=0; i<numOutputModules; i++) {
	    // Policy substrate
            layerInfo.layerSizes.push_back(Vector2<int>(numActions,1));
            layerInfo.layerIsInput.push_back(false);
            layerInfo.layerLocations.push_back(Vector3<float>(4*i,4+(4*numProcessingLevels),0));
            layerInfo.layerNames.push_back("Output" + boost::lexical_cast<std::string>(i));
	    // Preference substrate
            layerInfo.layerSizes.push_back(Vector2<int>(1,1)); // Single preference neuron
            layerInfo.layerIsInput.push_back(false);
            layerInfo.layerLocations.push_back(Vector3<float>((4*i) + 2,4+(4*numProcessingLevels),0));
            layerInfo.layerNames.push_back("Preference" + boost::lexical_cast<std::string>(i));
        }

        for (int i=0; i<numColors; ++i) {
	    // Schrum: connect to all processing layers
	    for (int j=0; j<numProcessingLayers; j++) {
            	layerInfo.layerAdjacencyList.push_back(std::pair<string,string>(
                                                       "Input" + boost::lexical_cast<std::string>(i),
                                                       "Processing0-" + boost::lexical_cast<std::string>(j)) );
	    }
        }

	// Schrum: Connect intermediate processing layers
	for (int i=0; i < (numProcessingLevels - 1); i++) {
	    for (int j=0; j<numProcessingLayers; j++) {
		for (int k=0; k<numProcessingLayers; k++) {
		    layerInfo.layerAdjacencyList.push_back(std::pair<string,string>(
                                                           "Processing" + boost::lexical_cast<std::string>(i)   + "-" + boost::lexical_cast<std::string>(j),
                                                           "Processing" + boost::lexical_cast<std::string>(i+1) + "-" + boost::lexical_cast<std::string>(k)) );
		}
	    }
	}

	// Schrum: All processing layers connect to all outputs and preference neurons
	for (int i=0; i<numProcessingLayers; i++) {
	    for (int j=0; j<numOutputModules; j++) {
                layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing" + boost::lexical_cast<std::string>(numProcessingLevels - 1) + "-" + boost::lexical_cast<std::string>(i),
										"Output"     + boost::lexical_cast<std::string>(j)));
                layerInfo.layerAdjacencyList.push_back(std::pair<string,string>("Processing" + boost::lexical_cast<std::string>(numProcessingLevels - 1) + "-" + boost::lexical_cast<std::string>(i),
										"Preference" + boost::lexical_cast<std::string>(j)));
	    }
	}

        layerInfo.normalize = true;
        layerInfo.useOldOutputNames = false;
        layerInfo.layerValidSizes = layerInfo.layerSizes;

        substrate.setLayerInfo(layerInfo);
	// Schrum: layers 0 through (numColors - 1) are for input, and
	//         there are (numProcessingLevels * numProcessingLayers) processing substrates
	// Schrum: This is merely the index of the first of several output substrates
        outputLayerIndx = numColors + (numProcessingLevels * numProcessingLayers);
    }

    // Schrum: override so that we can check preference neurons and pick an output from the right substrate
    Action AtariPixelPreferenceModulesExperiment::selectAction(NEAT::LayeredSubstrate<float>* substrate, int outputLayerIndx) {
        vector<int> max_inds;
        float max_val = -1e37;
        for (int i=0; i < numOutputModules; i++) {
	    // Schrum: preference neuron output: only neuron in substrate
	    // Schrum: Multiply by 2 because there are two neurons per module, +1 because preference comes after policy
            float output = substrate->getValue(Node(0,0,outputLayerIndx + (i*2) + 1));
            if (output == max_val)
                max_inds.push_back(i);
            else if (output > max_val) {
                max_inds.clear();
                max_inds.push_back(i);
                max_val = output;
            }
        }
        int action_indx = NEAT::Globals::getSingleton()->getRandom().getRandomInt(max_inds.size());
	// Schrum: perform action selection as usual on specified output policy substrate.
	//         Second argument is policy substrate corresponding to preference substrate with
	//         highest output.
        return AtariExperiment::selectAction(substrate, outputLayerIndx + (action_indx*2));
    }

    NEAT::GeneticPopulation* AtariPixelPreferenceModulesExperiment::createInitialPopulation(int populationSize) {
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
            	genes.push_back(GeneticNodeGene("Output_Input"   + boost::lexical_cast<std::string>(i) +
                                                "_Processing0-"  + boost::lexical_cast<std::string>(j),
                                                "NetworkOutputNode",1,false,
                                                ACTIVATION_FUNCTION_SIGMOID));
	    }
        }

	// Schrum: connect intermediate processing levels
	for (int i=0; i < (numProcessingLevels - 1); i++) {
	    for (int j=0; j<numProcessingLayers; j++) {
		for (int k=0; k<numProcessingLayers; k++) {
            	    genes.push_back(GeneticNodeGene("Output_Processing" + boost::lexical_cast<std::string>(i)  + "-" + boost::lexical_cast<std::string>(j) +
                                                          "_Processing" + boost::lexical_cast<std::string>(i+1)+ "-" + boost::lexical_cast<std::string>(k),
                                                    "NetworkOutputNode",1,false,
                                                    ACTIVATION_FUNCTION_SIGMOID));
		}
	    }
	}

	// Schrum: link each processing layer to each output layer and preference neuron
	for (int j=0; j<numProcessingLayers; j++) {
	    for (int k=0; k<numOutputModules; k++) {
                genes.push_back(GeneticNodeGene("Output_Processing" + boost::lexical_cast<std::string>(numProcessingLevels - 1) + "-" + boost::lexical_cast<std::string>(j) + 
						"_Output"           + boost::lexical_cast<std::string>(k),
						"NetworkOutputNode",1,false,
                                                ACTIVATION_FUNCTION_SIGMOID));
                genes.push_back(GeneticNodeGene("Output_Processing" + boost::lexical_cast<std::string>(numProcessingLevels - 1) + "-" + boost::lexical_cast<std::string>(j) + 
						"_Preference"       + boost::lexical_cast<std::string>(k),
						"NetworkOutputNode",1,false,
                                                ACTIVATION_FUNCTION_SIGMOID));
	    }
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
}
