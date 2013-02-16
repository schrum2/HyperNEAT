#ifndef HCUBE_ATARIINTRINSICEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIINTRINSICEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"

namespace HCUBE
{
    class SarsaLambda {
    public:
        //SarsaLambda() {};
        SarsaLambda(int numFeatures, int numActions, 
                    float gamma=.999, float alpha=.1, float epsilon=.1, float lambda=.3);
        ~SarsaLambda() {};
        void reset();
        int act(std::vector<bool>& currState, double lastActionReward);
        void printinfo();

    public:
        float gamma, alpha, epsilon, lambda; // Hyper-parameters
        int numFeatures, numActions;
        std::vector<float> w; // Weight vector
        std::vector<float> e; // Eligibility vector
        double oldQ, reward;

        int selectAction(std::vector<double>& qVals);
    };


    class AtariIntrinsicExperiment : public Experiment
    {
    protected:
        int substrate_width, substrate_height;
        IntMatrix screen_matrix;
        IntVect ram_content;
    
        ALEInterface ale;
        VisualProcessor* visProc;
        string rom_file;
        bool display_active;

        int numActions, numFeatures;
        int numObjClasses;

        SarsaLambda *agent;
        std::vector<bool> phi;

    public:
        NEAT::FastNetwork<double> substrate;
        map<Node,string> nameLookup; // Name lookup table

        void initializeExperiment(string rom_file);

        AtariIntrinsicExperiment(string _experimentName,int _threadID);
        virtual ~AtariIntrinsicExperiment() { if (agent) delete agent; };

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
        virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);
        void runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual);

        // Locates the object of each class on screen and populates their values to the
        // corresponding substrate layers
        void setSubstrateObjectValues(VisualProcessor& visProc);
        
        // Identifies the self agent on the relevant layer of substrate
        void setSubstrateSelfValue(VisualProcessor& visProc);

        // Takes the centroids of the objects locations and paints them onto the substrate
        void paintSubstrate(VisualProcessor& visProc, Prototype& proto, int substrateIndx);
    
        // Selects an action index 
        int selectAction(vector<double>& qVals);

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


#ifndef HCUBE_NOGUI
        virtual void createIndividualImage(wxDC &drawContext,shared_ptr<NEAT::GeneticIndividual> individual) {}

        //handleMousePress: returns true if the window needs to be refreshed
        virtual bool handleMousePress(wxMouseEvent& event,wxSize &bitmapSize) { return false; }

        //handleMouseMotion: returns true if the window needs to be refreshed
        virtual bool handleMouseMotion(wxMouseEvent& event,wxDC &temp_dc,shared_ptr<NEAT::GeneticIndividual> individual) {
            return false;
        }

        inline void drawPixel(int x,int y,int relativeResolution,wxColour** localBuffer,wxColour value)
        {
            //you want to draw a pixel at x,y if x,y was over 32, so multiply if it isn't

            int mody,modx;
            for (mody=0;mody<relativeResolution;mody++)
            {
                for (modx=0;modx<relativeResolution;modx++)
                {
                    localBuffer[(y*relativeResolution)+mody][(x*relativeResolution)+modx] = value;
                }
            }
        }
#endif
    };
}

#endif 
