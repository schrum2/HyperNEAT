/**
   This is FT-NEAT run with the pixel representation
 **/
#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariFTNeatNoiseExperiment.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    AtariFTNeatNoiseExperiment::AtariFTNeatNoiseExperiment(string _experimentName,int _threadID):
        AtariFTNeatExperiment(_experimentName, _threadID)
    {}

    void AtariFTNeatNoiseExperiment::initializeExperiment(string _rom_file) {
        initializeALE(_rom_file, false); // No screen processing necessary

        // Set the dimensions of our substrate to be that of the screen
        substrate_width = ale.screen_width / 10;
        substrate_height = ale.screen_height / 10;

        initializeTopology();
    }
    
    void AtariFTNeatNoiseExperiment::initializeTopology() {
        // Input Layer
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                Node node(x, y, 0);
                nameLookup[node] = (toString(x) + string("/") + toString(y) + string("/") + toString("0"));
            }
        }

        // Processing Layer
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                Node node(x, y, 1);
                nameLookup[node] = (toString(x) + string("/") + toString(y) + string("/") + toString("1"));
            }
        }
        
        // Output Layer - One node for each action
        for (int i=0; i<numActions; i++) {
            Node node(i,0,2);
            nameLookup[node] = (toString(i) + string("/") + toString("0") + string("/") + toString("2"));
        }
    }

    NEAT::GeneticPopulation* AtariFTNeatNoiseExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;
        vector<GeneticLinkGene> links;

        clock_t start = clock();
        vector<int> inputNodeIndexes;
        vector<int> hiddenNodeIndexes;
        vector<int> outputNodeIndexes;
        int nodeCounter = 0;
        // Input layer
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                Node node(x, y, 0);
                genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkSensor",0,false));
                inputNodeIndexes.push_back(nodeCounter);
                nodeCounter++;
            }
        }

        // Processing Layer
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                Node node(x, y, 1);
                genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkOutputNode",1,false,
                                            ACTIVATION_FUNCTION_SIGMOID));
                hiddenNodeIndexes.push_back(nodeCounter);
                nodeCounter++;
            }
        }

        // Output Layer
        for (int i=0; i<numActions; i++) {
            Node node(i, 0, 2);
            genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkOutputNode",1,false,
                                            ACTIVATION_FUNCTION_SIGMOID));
            outputNodeIndexes.push_back(nodeCounter);
            nodeCounter++;
        }

        // Bias node
        // genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false));
        // inputNodeIndexes.push_back(nodeCounter);
        // nodeCounter++;

        clock_t end = clock();
        cout << "Created " << nodeCounter << " nodes in " << float(end-start)/CLOCKS_PER_SEC << " seconds." << endl;

        start = clock();
        // [Links] Input --> Processing 
        for (int z=0; z<inputNodeIndexes.size(); z++) {
            for (int q=0; q<hiddenNodeIndexes.size(); q++) {
                int fromNodeIndex = inputNodeIndexes[z];
                int toNodeIndex   = hiddenNodeIndexes[q];
                links.push_back(GeneticLinkGene(genes[fromNodeIndex].getID(),genes[toNodeIndex].getID()));
            }
        }

        // [Links] Processing --> Output 
        for (int z=0; z<hiddenNodeIndexes.size(); z++) {
            for (int q=0; q<outputNodeIndexes.size(); q++) {
                int fromNodeIndex = hiddenNodeIndexes[z];
                int toNodeIndex   = outputNodeIndexes[q];
                links.push_back(GeneticLinkGene(genes[fromNodeIndex].getID(),genes[toNodeIndex].getID()));
            }
        }
        end = clock();
        cout << "Created " << links.size() << " links in " << float(end-start)/CLOCKS_PER_SEC << " seconds." << endl;

        for (int a=0; a<populationSize; a++) {
            shared_ptr<GeneticIndividual> individual(new GeneticIndividual(genes,links));
            for (int b=0;b<10;b++) {
                individual->testMutate();
            }
            population->addIndividual(individual);
        }

        cout << "Finished creating population\n";
        return population;
    }

    void AtariFTNeatNoiseExperiment::setSubstrateValues() {
        NEAT::Random& random = NEAT::Globals::getSingleton()->getRandom();
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                substrate.setValue(nameLookup[Node(x,y,0)], random.getRandomDouble());
            }
        }
    }
}
