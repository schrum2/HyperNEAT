/**
   This is FT-NEAT run with the object representation
 **/
#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariFTNeatExperiment.h"
#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    AtariFTNeatExperiment::AtariFTNeatExperiment(string _experimentName,int _threadID):
        Experiment(_experimentName,_threadID), visProc(NULL), rom_file(""),
        numActions(0), numObjClasses(0), display_active(false)
    {
    }

    void AtariFTNeatExperiment::initializeExperiment(string _rom_file) {
        initializeALE(_rom_file, true);
        initializeTopology();
    }

    void AtariFTNeatExperiment::initializeALE(string _rom_file, bool processScreen) {
        rom_file = _rom_file;

        substrate_width = 8;
        substrate_height = 10;

        // Check that rom exists and is readable
        ifstream file(rom_file.c_str());
        if (!file.good()) {
            cerr << "Unable to find or open rom file: \"" << rom_file << "\"" << endl;
            exit(-1);
        }

        // Initialize Atari Stuff
        if (!ale.loadROM(rom_file.c_str(), display_active, true)) {
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

    void AtariFTNeatExperiment::initializeTopology() {
        // One input layer for each object class, plus an extra one for the self object
        for (int i=0; i<=numObjClasses; ++i) {
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

    NEAT::GeneticPopulation* AtariFTNeatExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;
        vector<GeneticLinkGene> links;

        clock_t start = clock();
        vector<int> inputNodeIndexes;
        vector<int> hiddenNodeIndexes;
        vector<int> outputNodeIndexes;
        int nodeCounter = 0;
        // One input layer for each object class and an extra for the self object
        for (int i=0; i<=numObjClasses; ++i) {
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

    NEAT::GeneticPopulation* AtariFTNeatExperiment::convertPopulation(
        shared_ptr<NEAT::GeneticPopulation> CPPN_pop,
        shared_ptr<AtariExperiment> HyperNEAT_experiment)
    {
        shared_ptr<GeneticGeneration> HyperNEAT_lastGeneration = CPPN_pop->getGeneration();
        int numIndividuals = HyperNEAT_lastGeneration->getIndividualCount();

        // Initialize FT-NEAT population
        GeneticPopulation* FTNEAT_population = createInitialPopulation(numIndividuals);
        shared_ptr<GeneticGeneration> FTNEAT_lastGeneration = FTNEAT_population->getGeneration();
		
        // Copy all link values from HyperNEAT indivdual to FT-NEAT individual
        for (int individual = 0; individual<HyperNEAT_lastGeneration->getIndividualCount(); ++individual) {
            cout << "Converting indvidual " << individual << endl;
            shared_ptr<GeneticIndividual> HyperNEAT_individual = HyperNEAT_lastGeneration->getIndividual(individual);
            shared_ptr<GeneticIndividual> FTNEAT_individual    = FTNEAT_lastGeneration->getIndividual(individual);
            FTNEAT_individual->setFitness(HyperNEAT_individual->getFitness());
            HyperNEAT_experiment->substrate.populateSubstrate(HyperNEAT_individual);
            NEAT::LayeredSubstrate<float>* HyperNEAT_substrate = &HyperNEAT_experiment->substrate;
            convertIndividual(FTNEAT_individual, HyperNEAT_substrate);

            // cout << "Running HyperNEAT evaulation: " << endl;
            // NEAT::Globals::getSingleton()->initRandom();
            // HyperNEAT_experiment->populateSubstrate(HyperNEAT_individual);
            // HyperNEAT_experiment->runAtariEpisode(HyperNEAT_individual);
            // cout << "Running FT-NEAT evaluation: " << endl;
            // NEAT::Globals::getSingleton()->initRandom();
            // substrate = FTNEAT_individual->spawnFastPhenotypeStack<float>();
            // runAtariEpisode(FTNEAT_individual);
            // cout << "Done with individual" << endl;
        }
        return FTNEAT_population;
    }

    void AtariFTNeatExperiment::convertIndividual(shared_ptr<GeneticIndividual> FTNEAT_individual,
                                                  NEAT::LayeredSubstrate<float>* HyperNEAT_substrate)
                                                  
    {
        NEAT::FastLayeredNetwork<float>* HyperNEAT_network = HyperNEAT_substrate->getNetwork();

        // [Links] Input --> Processing 
        for (int i=0; i<=numObjClasses; ++i) {
            for (int x=0; x<substrate_width; x++) {
                for (int y=0; y<substrate_height; y++) {
                    Node fromNode(x, y, i);
                    int xmod = substrate_width*i+x; 
                    string sourceString(toString(xmod) + string("/") + toString(y) + string("/") + toString("0"));
                    GeneticNodeGene* sourceNode = FTNEAT_individual->getNode(sourceString);
                    // Processing Layer
                    for (int toY=0; toY<substrate_height; toY++) {
                        for (int toX=0; toX<substrate_width; toX++) {
                            Node toNode(toX, toY, numObjClasses+1);
                            string targetString(toString(toX) + string("/") + toString(toY) +
                                                string("/") + toString("1"));
                            GeneticNodeGene* targetNode = FTNEAT_individual->getNode(targetString);

                            float linkWeight = HyperNEAT_network->getLink(fromNode,toNode);

                            GeneticLinkGene* FTNEAT_link;
                            FTNEAT_link = FTNEAT_individual->getLink(sourceNode->getID(), targetNode->getID());
                            FTNEAT_link->setWeight(double(linkWeight));
                        }
                    }
                }
            }
        }

        // [Links] Processing --> Output
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                Node fromNode(x, y, numObjClasses+1);
                string sourceString(toString(x) + string("/") + toString(y) + string("/") + toString("1"));
                GeneticNodeGene* sourceNode = FTNEAT_individual->getNode(sourceString);

                // Output Layer
                for (int i=0; i<numActions; i++) {
                    Node toNode(i, 0, numObjClasses+2);

                    string targetString(toString(i) + string("/") + toString("0") + string("/") + toString("2"));
                    GeneticNodeGene* targetNode = FTNEAT_individual->getNode(targetString);

                    float linkWeight = HyperNEAT_network->getLink(fromNode,toNode);

                    GeneticLinkGene* FTNEAT_link;
                    FTNEAT_link = FTNEAT_individual->getLink(sourceNode->getID(), targetNode->getID());
                    FTNEAT_link->setWeight(double(linkWeight));
                }
            }
        }
    }

    void AtariFTNeatExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
    {
        shared_ptr<NEAT::GeneticIndividual> individual = group.front();
        evaluateIndividual(individual);
    }

    void AtariFTNeatExperiment::evaluateIndividual(shared_ptr<GeneticIndividual> individual)
    {
        individual->setFitness(0);
        substrate = individual->spawnFastPhenotypeStack<float>();
        runAtariEpisode(individual);
    }

    void AtariFTNeatExperiment::runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual) {
        // Reset the game
        ale.reset_game();
        
        while (!ale.game_over()) {
            // Set value of all nodes to zero
            substrate.reinitialize(); 
            substrate.dummyActivation();

            setSubstrateValues();

            // Propagate values through the ANN
            // This is necessary to fully propagate through the different layers
            substrate.updateFixedIterations(2);

            //printLayerInfo();

            // Choose which action to take
            Action action = selectAction(*visProc);
            ale.act(action);
        }
        cout << "Game ended in " << ale.frame << " frames with score " << ale.game_score << endl;
 
        // Give the reward to the agent
        individual->reward(ale.game_score);
    }

    void AtariFTNeatExperiment::setSubstrateValues() {
        // Set substrate value for all objects (of a certain size)
        setSubstrateObjectValues(*visProc);

        // Set substrate value for self
        setSubstrateSelfValue(*visProc);
    }

    void AtariFTNeatExperiment::setSubstrateObjectValues(VisualProcessor& visProc) {
        for (int i=0; i<visProc.manual_obj_classes.size(); i++) {
            Prototype& proto = visProc.manual_obj_classes[i];
            paintSubstrate(visProc, proto, i);
        }
    }

    void AtariFTNeatExperiment::paintSubstrate(VisualProcessor& visProc, Prototype& proto, int substrateIndx) {
        // Assign values to each of the objects
        float assigned_value = 1.0;
        for (set<long>::iterator it=proto.obj_ids.begin(); it!=proto.obj_ids.end(); it++) {
            long obj_id = *it;
            assert(visProc.composite_objs.find(obj_id) != visProc.composite_objs.end());
            point obj_centroid = visProc.composite_objs[obj_id].get_centroid();
            int adj_x = obj_centroid.x * substrate_width / visProc.screen_width;
            int adj_y = obj_centroid.y * substrate_height / visProc.screen_height;
            substrate.setValue(nameLookup[Node(substrate_width*substrateIndx+adj_x,adj_y,0)], assigned_value);
        }
    }

    void AtariFTNeatExperiment::setSubstrateSelfValue(VisualProcessor& visProc) {
        if (!visProc.found_self())
            return;
        paintSubstrate(visProc, visProc.manual_self, numObjClasses);
    }

    Action AtariFTNeatExperiment::selectAction(VisualProcessor& visProc) {
        vector<int> max_inds;
        float max_val = -1e37;
        for (int i=0; i < numActions; i++) {
            float output = substrate.getValue(nameLookup[Node(i,0,2)]);
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

    double AtariFTNeatExperiment::gauss2D(double x, double y, double A, double mu_x, double mu_y,
                                    double sigma_x, double sigma_y)
    {
        return A * exp(-1.0 * ((x-mu_x) * (x-mu_x) / 2.0 * sigma_x * sigma_x + (y-mu_y) * (y-mu_y) / 2.0 * sigma_y * sigma_y));
    }

    void AtariFTNeatExperiment::printLayerInfo() {
        for (int i=0; i<=numObjClasses; i++) {
            printf("Obj Class %d Substrate:\n",i);
            for (int y=0; y<substrate_height; y++) {
                for (int x=0; x<substrate_width; x++) {
                    float output = substrate.getValue(nameLookup[Node(i*substrate_width+x,y,0)]);
                    printf("%1.1f ",output);
                }
                printf("\n");
            }
            printf("\n");
        }
        printf("Processing Layer:\n");
        for (int y=0; y<substrate_height; y++) {
            for (int x=0; x<substrate_width; x++) {
                float output = substrate.getValue(nameLookup[Node(x,y,1)]);
                printf("%1.1f ",output);
            }
            printf("\n");
        }
        printf("\n");
        printf("Output Layer:\n");
        for (int i=0; i < numActions; i++) {
            float output = substrate.getValue(nameLookup[Node(i,0,2)]);
            printf("%1.1f ",output);
        }
        printf("\n");
        cin.get();
    }

    Experiment* AtariFTNeatExperiment::clone()
    {
        AtariFTNeatExperiment* experiment = new AtariFTNeatExperiment(*this);
        return experiment;
    }
}
