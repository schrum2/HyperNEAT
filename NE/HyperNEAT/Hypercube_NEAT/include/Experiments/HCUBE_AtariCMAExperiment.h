#ifndef HCUBE_ATARICMAEXPERIMENT_H_INCLUDED
#define HCUBE_ATARICMAEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"
#include "Experiments/HCUBE_AtariExperiment.h"
#include <boost/filesystem.hpp>

namespace HCUBE
{
    class AtariCMAExperiment : public Experiment
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

    public:
        int generationNumber;
        int individualToEvaluate;
        boost::filesystem::path resultsPath;

        NEAT::FastNetwork<float> substrate;
        map<Node,string> nameLookup; // Name lookup table

        // Sets the result path from the genration path
        void setResultsPath(string generationPath);

        virtual void initializeExperiment(string rom_file);

        // Sets up the ALE interface and loads the rom. ProcessScreen enables/disables object detection
        virtual void initializeALE(string rom_file, bool processScreen);
        // Creates the layers and layerinfo
        virtual void initializeTopology();

        AtariCMAExperiment(string _experimentName,int _threadID);
        virtual ~AtariCMAExperiment() {};

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);

        virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);
        virtual void evaluateIndividual(shared_ptr<NEAT::GeneticIndividual> individual);
        void runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual);

        // Sets the activations on the input layer of the substrates
        virtual void setSubstrateValues();

        // Locates the object of each class on screen and populates their values to the
        // corresponding substrate layers
        void setSubstrateObjectValues(VisualProcessor& visProc);
        
        // Identifies the self agent on the relevant layer of substrate
        void setSubstrateSelfValue(VisualProcessor& visProc);

        // Takes the centroids of the objects locations and paints them onto the substrate
        void paintSubstrate(VisualProcessor& visProc, Prototype& proto, int substrateIndx);
    
        // Selects an action based on the output layer of the network
        Action selectAction(VisualProcessor& visProc);

        // Prints a visual representation of each input layer
        void printLayerInfo();

        double gauss2D(double x, double y, double A, double mu_x, double mu_y, double sigma_x,
                       double sigma_y);

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

        virtual Experiment* clone();
        virtual void resetGenerationData(shared_ptr<NEAT::GeneticGeneration> generation) {}
        virtual void addGenerationData(shared_ptr<NEAT::GeneticGeneration> generation,shared_ptr<NEAT::GeneticIndividual> individual) {}
    };
}

#endif 
