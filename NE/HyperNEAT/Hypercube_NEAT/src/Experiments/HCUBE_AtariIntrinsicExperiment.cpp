#include "HCUBE_Defines.h"

#include "Experiments/HCUBE_AtariIntrinsicExperiment.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace NEAT;

namespace HCUBE
{
    SarsaLambda::SarsaLambda(int numFeatures, int numActions,
                             float gamma, float alpha, float epsilon, float lambda):
        numFeatures(numFeatures), numActions(numActions), gamma(gamma), alpha(alpha), epsilon(epsilon), lambda(lambda)
    {
        // Initialize the SARSA(Lambda) vectors
        for (int i=0; i<numFeatures*numActions; i++) {
            w.push_back(0);
            e.push_back(0);
        }
    }

    void SarsaLambda::reset() {
        // Reset eligibility vector
        for (int i=0; i<numFeatures*numActions; i++)
            e[i] = 0;
        reward = 0;
    }

    int SarsaLambda::act(std::vector<bool>& currState, double lastActionReward) {
        assert(currState.size() == numFeatures);

        vector<double> qVals;
        for (int a=0; a<numActions; a++) {
            double Q_a = 0;
            for (int i=0; i<numFeatures; i++)
                if (currState[i])
                    Q_a += w[a*numFeatures+i];
            qVals.push_back(Q_a);
        }

        int action = selectAction(qVals);
        double Q = qVals[action];

        // Compute bellman error 
        double delta = lastActionReward + (gamma * Q) - oldQ;

        // Update the weights
        for (int i=0; i<numFeatures*numActions; i++) {
            w[i] += alpha * delta * e[i];
        }
                
        // Decay the eligibility traces
        for (int i=0; i<numFeatures*numActions; i++) {
            e[i] *= gamma * lambda;
        }

        // Set the active features' eligibility traces to 1
        for (int i=0; i<numFeatures; i++) {
            if (currState[i])
                e[numFeatures*action + i] = 1;
        }

        oldQ = Q;
        return action;
    }

    int SarsaLambda::selectAction(vector<double>& qVals) {
        if (double(rand())/RAND_MAX < epsilon) {
            return rand() % numActions;
        } else {
            vector<int> max_inds;
            double max_val = -1e37;
            for (int i=0; i < numActions; i++) {
                if (qVals[i] == max_val)
                    max_inds.push_back(i);
                else if (qVals[i] > max_val) {
                    max_inds.clear();
                    max_inds.push_back(i);
                    max_val = qVals[i];
                }
            }
            return max_inds[rand() % max_inds.size()];
        }
    }

    void SarsaLambda::printinfo() {
        printf("Gamma %f alpha %f epsilon %f lambda %f\n",gamma,alpha,epsilon,lambda);
        cout << "Weight Vec: ";
        for (int i=0; i<numFeatures*numActions; i++) {
            printf("%1.2f ",w[i]);
            if (i%numFeatures == 0)
                printf("\n");
        }
        cout << endl;
    }


    AtariIntrinsicExperiment::AtariIntrinsicExperiment(string _experimentName,int _threadID):
        Experiment(_experimentName,_threadID), visProc(NULL), rom_file(""),
        numActions(0), numObjClasses(0), display_active(false), agent(NULL)
    {
    }

    void AtariIntrinsicExperiment::initializeExperiment(string _rom_file) {
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
        numFeatures = substrate_width * substrate_height * (numObjClasses + 1);

        for (int i=0; i<numFeatures; i++)
            phi.push_back(0);

        if (agent) delete agent;
        agent = new SarsaLambda(numFeatures, numActions);

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
        
        // Single output node computes the reward
        Node node(0,0,2);
        nameLookup[node] = string("0/0/2");
    }

    NEAT::GeneticPopulation* AtariIntrinsicExperiment::createInitialPopulation(int populationSize) {
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
        Node node(0,0,2);
        genes.push_back(GeneticNodeGene(nameLookup[node],"NetworkOutputNode",1,false, ACTIVATION_FUNCTION_SIGMOID));

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

    void AtariIntrinsicExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
    {
        shared_ptr<NEAT::GeneticIndividual> individual = group.front();
        individual->setFitness(0);

        substrate = individual->spawnFastPhenotypeStack<double>();

        runAtariEpisode(individual);
    }

    void AtariIntrinsicExperiment::runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual) {
        agent->printinfo();
        int numEpisodes = 10000;
        double totalScore = 0;
        for (int episode = 0; episode < numEpisodes; episode++) {
            double reward = 0;
            ale.reset_game();
            agent->reset();
        
            while (!ale.game_over()) {
                for (int i=0; i<numFeatures; i++)
                    phi[i] = false;

                // Set value of all nodes to zero
                substrate.reinitialize(); 
                substrate.dummyActivation();

                // Set substrate value for all objects (of a certain size)
                setSubstrateObjectValues(*visProc);

                // Set substrate value for self
                setSubstrateSelfValue(*visProc);

                // Propagate values through the ANN
                substrate.update();

                // cout << "Phi: ";
                // for (int i=0; i<numFeatures; i++)
                //     cout << phi[i] << " ";
                // cout << endl;
                // printLayerInfo();

                int action_indx = agent->act(phi, reward);
                Action action = ale.legal_actions[action_indx];
                reward = ale.act(action);
            }
            cout << "Episode " << episode << " ended in " << ale.frame << " frames with score " << ale.game_score << endl;
            totalScore += ale.game_score;
        }

        // Give the reward to the agent
        float avgScore = totalScore / float(numEpisodes);
        individual->reward(avgScore);
    }

    void AtariIntrinsicExperiment::setSubstrateObjectValues(VisualProcessor& visProc) {
        for (int i=0; i<visProc.manual_obj_classes.size(); i++) {
            Prototype& proto = visProc.manual_obj_classes[i];
            paintSubstrate(visProc, proto, i);
        }
    }

    void AtariIntrinsicExperiment::paintSubstrate(VisualProcessor& visProc, Prototype& proto, int substrateIndx) {
        // Assign values to each of the objects
        float assigned_value = 1.0;
        for (set<long>::iterator it=proto.obj_ids.begin(); it!=proto.obj_ids.end(); it++) {
            long obj_id = *it;
            assert(visProc.composite_objs.find(obj_id) != visProc.composite_objs.end());
            point obj_centroid = visProc.composite_objs[obj_id].get_centroid();
            int adj_x = obj_centroid.x * substrate_width / visProc.screen_width;
            int adj_y = obj_centroid.y * substrate_height / visProc.screen_height;
            substrate.setValue(nameLookup[Node(substrate_width*substrateIndx+adj_x,adj_y,0)], assigned_value);
            // Set the phi-feature to true
            phi[(substrate_width*substrate_height*substrateIndx) + (substrate_width*adj_y) + adj_x] = true;
        }
    }

    void AtariIntrinsicExperiment::setSubstrateSelfValue(VisualProcessor& visProc) {
        if (!visProc.found_self())
            return;
        paintSubstrate(visProc, visProc.manual_self, numObjClasses);
    }

    int AtariIntrinsicExperiment::selectAction(vector<double>& qVals) {
        // if (NEAT::Globals::getSingleton()->getRandom().getRandomDouble() <= epsilon) {
        //     cout << "Selecting Random Action" << endl;
        //     return NEAT::Globals::getSingleton()->getRandom().getRandomInt(numActions);
        // } else {
        //     cout << "Q-Vals: ";
        //     vector<int> max_inds;
        //     double max_val = -1e37;
        //     for (int i=0; i < numActions; i++) {
        //         cout << qVals[i] << " ";
        //         if (qVals[i] == max_val)
        //             max_inds.push_back(i);
        //         else if (qVals[i] > max_val) {
        //             max_inds.clear();
        //             max_inds.push_back(i);
        //             max_val = qVals[i];
        //         }
        //     }
        //     cout << endl;
        //     int max_indx = NEAT::Globals::getSingleton()->getRandom().getRandomInt(max_inds.size());
        //     int action_indx = max_inds[max_indx]; //ale.legal_actions[max_inds[action_indx]];
        //     return action_indx;
        // }
    }

    double AtariIntrinsicExperiment::gauss2D(double x, double y, double A, double mu_x, double mu_y,
                                    double sigma_x, double sigma_y)
    {
        return A * exp(-1.0 * ((x-mu_x) * (x-mu_x) / 2.0 * sigma_x * sigma_x + (y-mu_y) * (y-mu_y) / 2.0 * sigma_y * sigma_y));
    }

    void AtariIntrinsicExperiment::printLayerInfo() {
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
    }

    Experiment* AtariIntrinsicExperiment::clone()
    {
        AtariIntrinsicExperiment* experiment = new AtariIntrinsicExperiment(*this);
        return experiment;
    }
}
