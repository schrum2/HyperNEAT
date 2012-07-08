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

  public:
    NEAT::LayeredSubstrate<float> substrates[2];
    int currentSubstrateIndex;
    shared_ptr<const NEAT::GeneticIndividual> substrateIndividuals[2];
    //shared_ptr<NEAT::GeneticIndividual> currentSubstrateIndividual;

    void initializeExperiment(string rom_file);

    AtariExperiment(string _experimentName,int _threadID);
    virtual ~AtariExperiment() { if (visProc) delete visProc; }

    virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
    virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);
    void runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual);
    void printLayerInfo(NEAT::LayeredSubstrate<float>* substrate);

    // Locates the object of each class on screen and populates their values to the
    // corresponding substrate layers
    void setSubstrateObjectValues(VisualProcessor& visProc,
                                  NEAT::LayeredSubstrate<float>* substrate);
    // Identifies the self agent on the relevant layer of substrate
    void setSubstrateSelfValue(VisualProcessor& visProc,
                                  NEAT::LayeredSubstrate<float>* substrate);
    // Selects an action based on the output layer of the network
    Action selectAction(VisualProcessor& visProc,
                                  NEAT::LayeredSubstrate<float>* substrate);

    double gauss2D(double x, double y, double A, double mu_x, double mu_y, double sigma_x,
                   double sigma_y);
    void populateSubstrate(shared_ptr<NEAT::GeneticIndividual> individual, int substrateNum=0);
    virtual void processIndividualPostHoc(shared_ptr<NEAT::GeneticIndividual> individual);
    void preprocessIndividual(shared_ptr<NEAT::GeneticGeneration> generation,
                              shared_ptr<NEAT::GeneticIndividual> individual);

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

#endif // HCUBE_XOREXPERIMENT_H_INCLUDED
