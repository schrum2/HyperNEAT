#ifndef HCUBE_ATARIEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "../../../ale_v0.1/test.h"

namespace HCUBE
{
  class AtariExperiment : public Experiment
  {
  protected:
    int substrate_width, substrate_height;
    IntMatrix screen_matrix;
    IntVect ram_content;
    
    InternalController* controller;
    SelfDetectionAgent* self_detection_agent;
    GameSettings* game_settings;
    string rom_file;
    bool display_active;

  public:
    NEAT::LayeredSubstrate<float> substrate;
    shared_ptr<NEAT::GeneticIndividual> currentSubstrateIndividual;

    AtariExperiment(string _experimentName,int _threadID);
    virtual ~AtariExperiment() {}

    virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
    virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);
    void runAtariEpisode(shared_ptr<NEAT::GeneticIndividual> individual);
    void populateSubstrate(shared_ptr<NEAT::GeneticIndividual> individual);
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
