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
        
	// Schrum: Has to be overridden to select from output modules
        virtual Action selectAction(NEAT::LayeredSubstrate<float>* substrate, int outputLayerIndx);

	// Schrum: new to this experiment: how many output modules?
	void setOutputModules(int num);
    protected:
	int numOutputModules; // Schrum: more than one output module: multimodal 
    };
}

#endif // HCUBE_ATARIPIXELPREFERENCEMODULESEXPERIMENT_H_INCLUDED
