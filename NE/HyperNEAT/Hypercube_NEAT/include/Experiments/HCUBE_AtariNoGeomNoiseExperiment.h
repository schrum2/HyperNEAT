#ifndef HCUBE_ATARINOGEOMNOISEEXPERIMENT_H_INCLUDED
#define HCUBE_ATARINOGEOMNOISEEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "HCUBE_AtariNoGeomExperiment.h"

namespace HCUBE
{
    class AtariNoGeomNoiseExperiment : public AtariNoGeomExperiment
    {
    public:
        AtariNoGeomNoiseExperiment(string _experimentName,int _threadID);
        virtual ~AtariNoGeomNoiseExperiment() {};

        virtual void initializeExperiment(string rom_file);

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
        virtual void setSubstrateValues();
        virtual void initializeTopology();
    };
}

#endif 
