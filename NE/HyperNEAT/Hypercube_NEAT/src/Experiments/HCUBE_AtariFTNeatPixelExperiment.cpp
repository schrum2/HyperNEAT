/**
   This is FT-NEAT run with the pixel representation
 **/
#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariFTNeatPixelExperiment.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    uInt32 AtariFTNeatPixelExperiment::eightBitPallete [256] = {
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


    AtariFTNeatPixelExperiment::AtariFTNeatPixelExperiment(string _experimentName,int _threadID):
        AtariFTNeatExperiment(_experimentName, _threadID)
    {}

    void AtariFTNeatPixelExperiment::initializeExperiment(string _rom_file) {
        initializeALE(_rom_file, false); // No screen processing necessary

        // Set the dimensions of our substrate to be that of the screen
        substrate_width = ale.screen_width / 10;
        substrate_height = ale.screen_height / 10;

        initializeTopology();
    }
    
    void AtariFTNeatPixelExperiment::initializeTopology() {
        for (int i=0; i<numColors; ++i) {
            for (int y=0; y<substrate_height; y++) {
                for (int x=0; x<substrate_width; x++) {
                    int xmod = substrate_width*i+x; 
                    Node node(xmod, y, 0);
                    nameLookup[node] = (toString(xmod) + string("/") + toString(y) + string("/") + toString("0"));
                }
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

    NEAT::GeneticPopulation* AtariFTNeatPixelExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;
        vector<GeneticLinkGene> links;

        clock_t start = clock();
        vector<int> inputNodeIndexes;
        vector<int> hiddenNodeIndexes;
        vector<int> outputNodeIndexes;
        int nodeCounter = 0;
        // One input layer for each object class and an extra for the self object
        for (int i=0; i<numColors; ++i) {
            for (int y=0; y<substrate_height; y++) {
                for (int x=0; x<substrate_width; x++) {
                    int xmod = substrate_width*i+x; 
                    Node node(xmod, y, 0);
                    genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkSensor",0,false));
                    inputNodeIndexes.push_back(nodeCounter);
                    nodeCounter++;
                }
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

    void AtariFTNeatPixelExperiment::setSubstrateValues() {
        for (int y=0; y<ale.screen_height; y++) {
            for (int x=0; x<ale.screen_width; x++) {
                int substrate_y = min(int((y / float(ale.screen_height)) * substrate_width), substrate_height-1);
                int substrate_x = min(int((x / float(ale.screen_width)) * substrate_width), substrate_width-1);
                uInt32 eightBitVal = eightBitPallete[ale.screen_matrix[y][x]];
                assert(eightBitVal < numColors);
                assert(substrate_x < substrate_width);
                assert(substrate_y < substrate_height);
                //substrate->setValue((Node(substrate_x, substrate_y, eightBitVal)), 1.0);
                substrate.setValue(nameLookup[Node(substrate_width*eightBitVal+substrate_x,substrate_y,0)], 1.0);
            }
        }
    }
}
