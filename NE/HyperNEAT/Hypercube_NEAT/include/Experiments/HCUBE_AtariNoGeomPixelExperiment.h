#ifndef HCUBE_ATARINOGEOMPIXELEXPERIMENT_H_INCLUDED
#define HCUBE_ATARINOGEOMPIXELEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "ale_interface.hpp"
#include "HCUBE_AtariNoGeomExperiment.h"
#include "common/visual_processor.h"

namespace HCUBE
{
    class AtariNoGeomPixelExperiment : public AtariNoGeomExperiment
    {
    public:
        AtariNoGeomPixelExperiment(string _experimentName,int _threadID);
        virtual ~AtariNoGeomPixelExperiment() {};

        virtual void initializeExperiment(string rom_file);

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);
        virtual void setSubstrateValues();
        virtual void initializeTopology();

    protected:
        // The number of different colors in the color representation
        const static int numColors = 8;

        static uInt32 eightBitPallete[256];
    };
}

#endif 
