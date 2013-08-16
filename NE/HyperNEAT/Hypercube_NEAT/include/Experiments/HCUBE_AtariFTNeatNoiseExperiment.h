#ifndef HCUBE_ATARIFTNEATNOISEEXPERIMENT_H_INCLUDED
#define HCUBE_ATARIFTNEATNOISEEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"
#include "HCUBE_AtariFTNeatExperiment.h"

namespace HCUBE
{
    class AtariFTNeatNoiseExperiment : public AtariFTNeatExperiment
    {
    public:
        AtariFTNeatNoiseExperiment(string _experimentName,int _threadID);

        virtual void initializeExperiment(string rom_file);
        virtual void initializeTopology();
        virtual void setSubstrateValues();

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
    };
}

#endif 
