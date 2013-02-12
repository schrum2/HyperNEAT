#ifndef HCUBE_ATARIEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"

namespace HCUBE
{
    class AtariExperiment : public Experiment
    {
    protected:
        int substrate_width, substrate_height;
        IntMatrix screen_matrix;
        IntVect ram_content;
    
        ALEInterface ale;
        VisualProcessor* visProc;
        string rom_file;
        bool display_active;

        int numActions;
        int numObjClasses;

    public: // TODO: Make this protected 
        NEAT::LayeredSubstrate<float> substrate;

    public: 
        AtariExperiment(string _experimentName,int _threadID);
        virtual ~AtariExperiment() {};

        // Initializes ale and toplogy
        virtual void initializeExperiment(string rom_file);
        // Sets up the ALE interface and loads the rom
        virtual void initializeALE(string rom_file);
        // Creates the layers and layerinfo
        virtual void initializeTopology();

        // Creates the population of individuals
        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
        // Evaluates the individual at the front of the group
        virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);
        // Runs the atari episode using the specified individual
        virtual float runAtariEpisode(NEAT::LayeredSubstrate<float>* substrate);
        // Prints the activations at each layer of the substrate
        virtual void printLayerInfo(NEAT::LayeredSubstrate<float>* substrate);

        // Locates the object of each class on screen and populates their values to the
        // corresponding substrate layers
        virtual void setSubstrateObjectValues(VisualProcessor& visProc,
                                      NEAT::LayeredSubstrate<float>* substrate);
        // Identifies the self agent on the relevant layer of substrate
        virtual void setSubstrateSelfValue(VisualProcessor& visProc,
                                   NEAT::LayeredSubstrate<float>* substrate);

        // Takes the centroids of the objects locations and paints them onto the substrate
        virtual void paintSubstrate(VisualProcessor& visProc, Prototype& proto,
                                    NEAT::LayeredSubstrate<float>* substrate, int substrateIndx);
    
        // Selects an action based on the output layer of the network
        virtual Action selectAction(VisualProcessor& visProc,
                            NEAT::LayeredSubstrate<float>* substrate);

        // Creates a gaussian blur around an object
        static double gauss2D(double x, double y, double A,
                              double mu_x, double mu_y,
                              double sigma_x, double sigma_y);


        // Methods Inherited from HCUBE_Experiment
        void processIndividualPostHoc(shared_ptr<NEAT::GeneticIndividual> individual) {};
        void preprocessIndividual(shared_ptr<NEAT::GeneticGeneration> generation,
                                  shared_ptr<NEAT::GeneticIndividual> individual) {};
        virtual bool performUserEvaluations() { return false; }
        void set_rom(string new_rom) { rom_file = new_rom; }
        void setDisplayScreen(bool disp) { display_active = disp; }
        virtual inline bool isDisplayGenerationResult() { return displayGenerationResult; }
        virtual inline void setDisplayGenerationResult(bool _displayGenerationResult) {
            displayGenerationResult=_displayGenerationResult;
        }
        virtual inline void toggleDisplayGenerationResult() {
            displayGenerationResult=!displayGenerationResult;
        }
        virtual Experiment* clone() {
            AtariExperiment* experiment = new AtariExperiment(*this);
            return experiment;
        };
        virtual void resetGenerationData(shared_ptr<NEAT::GeneticGeneration> generation) {}
        virtual void addGenerationData(shared_ptr<NEAT::GeneticGeneration> generation,shared_ptr<NEAT::GeneticIndividual> individual) {}
    };
}

#endif // HCUBE_ATARI_EXPERIMENT
