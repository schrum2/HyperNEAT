#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariNoGeomExperiment.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    AtariNoGeomExperiment::AtariNoGeomExperiment(string _experimentName,int _threadID):
        Experiment(_experimentName,_threadID), visProc(NULL), rom_file(""),
        numActions(0), numObjClasses(0), display_active(false)
    {
    }

    void AtariNoGeomExperiment::initializeExperiment(string _rom_file) {
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
        if (!ale.loadROM(rom_file.c_str(), display_active, true)) {
            cerr << "Ale had problem loading rom..." << endl;
            exit(-1);
        }
        numActions = ale.legal_actions.size();

        // Load the visual processing framework
        visProc = ale.visProc;
        numObjClasses = visProc->manual_obj_classes.size();
        if (numObjClasses <= 0) {
            cerr << "No object classes found. Make sure there is an images directory containg class images." << endl;
            exit(-1);
        }

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
        
        // Output Layer - One node for each action
        for (int i=0; i<numActions; i++) {
            Node node(i,0,2);
            nameLookup[node] = (toString(i) + string("/") + toString("0") + string("/") + toString("2"));
        }
    }

    NEAT::GeneticPopulation* AtariNoGeomExperiment::createInitialPopulation(int populationSize) {
        GeneticPopulation *population = new GeneticPopulation();
        vector<GeneticNodeGene> genes;

        // One input layer for each object class and an extra for the self object
        for (int i=0; i<=numObjClasses; ++i) {
            for (int y=0; y<substrate_height; y++) {
                for (int x=0; x<substrate_width; x++) {
                    int xmod = substrate_width*i+x; 
                    Node node(xmod, y, 0);
                    genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkSensor",0,false));
                }
            }
        }

        // Output Layer
        for (int i=0; i<numActions; i++) {
            Node node(i, 0, 2);
            genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkOutputNode",1,false,
                                            ACTIVATION_FUNCTION_SIGMOID));
        }

        // Bias node
        genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false));

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

    void AtariNoGeomExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
    {
        shared_ptr<NEAT::GeneticIndividual> individual = group.front();
        individual->setFitness(0);

        // Print the individual. This is rarely useful...
        //individual->print();

        substrate = individual->spawnFastPhenotypeStack<double>();

        runAtariEpisode(individual);
    }

    void AtariNoGeomExperiment::runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual) {
        // Reset the game
        ale.reset_game();
        
        while (!ale.game_over()) {
            // Set value of all nodes to zero
            substrate.reinitialize(); 
            substrate.dummyActivation();

            // Set substrate value for all objects (of a certain size)
            setSubstrateObjectValues(*visProc);

            // Set substrate value for self
            setSubstrateSelfValue(*visProc);

            // Propagate values through the ANN
            substrate.update();

            // Choose which action to take
            Action action = selectAction(*visProc);
            ale.act(action);
        }
        cout << "Game ended in " << ale.frame << " frames with score " << ale.game_score << endl;
 
        // Give the reward to the agent
        individual->reward(ale.game_score);
    }

    void AtariNoGeomExperiment::setSubstrateObjectValues(VisualProcessor& visProc) {
        for (int i=0; i<visProc.manual_obj_classes.size(); i++) {
            Prototype& proto = visProc.manual_obj_classes[i];
            paintSubstrate(visProc, proto, i);
        }
    }

    void AtariNoGeomExperiment::paintSubstrate(VisualProcessor& visProc, Prototype& proto, int substrateIndx) {
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

    void AtariNoGeomExperiment::setSubstrateSelfValue(VisualProcessor& visProc) {
        if (!visProc.found_self())
            return;
        paintSubstrate(visProc, visProc.manual_self, numObjClasses);
    }

    Action AtariNoGeomExperiment::selectAction(VisualProcessor& visProc) {
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
        //int action_indx = choice(&max_inds);
        return ale.legal_actions[max_inds[action_indx]];
    }

    double AtariNoGeomExperiment::gauss2D(double x, double y, double A, double mu_x, double mu_y,
                                    double sigma_x, double sigma_y)
    {
        return A * exp(-1.0 * ((x-mu_x) * (x-mu_x) / 2.0 * sigma_x * sigma_x + (y-mu_y) * (y-mu_y) / 2.0 * sigma_y * sigma_y));
    }

    Experiment* AtariNoGeomExperiment::clone()
    {
        AtariNoGeomExperiment* experiment = new AtariNoGeomExperiment(*this);
        return experiment;
    }
}
