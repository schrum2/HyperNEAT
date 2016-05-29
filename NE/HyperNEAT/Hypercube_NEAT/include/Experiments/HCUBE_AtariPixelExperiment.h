#ifndef HCUBE_ATARIPIXELEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIPIXELEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "HCUBE_AtariExperiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"

namespace HCUBE
{
    class AtariPixelExperiment : public AtariExperiment
    {
    public: 
        AtariPixelExperiment(string _experimentName, int _threadID);
        virtual ~AtariPixelExperiment() {};

        virtual void initializeExperiment(string rom_name);
        virtual void initializeTopology();

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
        virtual void setSubstrateValues(NEAT::LayeredSubstrate<float>* substrate);

	// Schrum: Needed to set the processing layers
	void setProcessingLayers(int num);
	// Schrum: Needed to set the processing levels
	void setProcessingLevels(int num);
    protected:
        // The number of different colors in the color representation
        const static int numColors = 8;

        static uInt32 eightBitPallete[256];

	int numProcessingLayers; // Schrum: this actually defines the number of processing substrates per level
	int numProcessingLevels; // Schrum: this is the depth/number of levels worth of processing layers
    };
}

#endif // HCUBE_ATARIPIXELEXPERIMENT_H_INCLUDED
