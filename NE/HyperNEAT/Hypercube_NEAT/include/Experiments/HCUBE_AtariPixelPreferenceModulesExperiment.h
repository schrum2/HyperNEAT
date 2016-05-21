#ifndef HCUBE_ATARIPIXELPREFERENCEMODULESEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIPIXELPREFERENCEMODULESEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "HCUBE_AtariPixelExperiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"

namespace HCUBE
{
    class AtariPixelPreferenceModulesExperiment : public AtariPixelExperiment
    {
    public: 
        AtariPixelPreferenceModulesExperiment(string _experimentName, int _threadID);
        virtual ~AtariPixelPreferenceModulesExperiment() {};

        virtual void initializeTopology();

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
        
	// Schrum: shouldn't need to change these
        //virtual void initializeExperiment(string rom_name);
	//virtual void setSubstrateValues(NEAT::LayeredSubstrate<float>* substrate);
	//void setProcessingLayers(int num);

	// Schrum: Has to be overridden to select from output modules
        virtual Action selectAction(NEAT::LayeredSubstrate<float>* substrate, int outputLayerIndx);

	// Schrum: new to this experiment: how many output modules?
	void setOutputModules(int num);
    protected:
	// Schrum: This don't change, so I think I can leave them out
        //const static int numColors = 8;
        //static uInt32 eightBitPallete[256];
	//int numProcessingLayers; 

	int numOutputModules; // Schrum: more than one output module: multimodal 
    };
}

#endif // HCUBE_ATARIPIXELPREFERENCEMODULESEXPERIMENT_H_INCLUDED
