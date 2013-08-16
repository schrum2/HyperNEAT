#ifndef HCUBE_ATARIFTNEATPIXELEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIFTNEATPIXELEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"
#include "HCUBE_AtariFTNeatExperiment.h"
#include "Experiments/HCUBE_AtariExperiment.h"

namespace HCUBE
{
    class AtariFTNeatPixelExperiment : public AtariFTNeatExperiment
    {
    public:
        AtariFTNeatPixelExperiment(string _experimentName,int _threadID);
        virtual ~AtariFTNeatPixelExperiment() {};

        void initializeExperiment(string rom_file);
        virtual void initializeTopology();
        virtual void setSubstrateValues();

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);

    protected:
        // The number of different colors in the color representation
        const static int numColors = 8;

        static uInt32 eightBitPallete[256];
    };
}

#endif 
