#ifndef HCUBE_ATARINOISEEXPERIMENT_H_INCLUDED
#define HCUBE_ATARINOISEEXPERIMENT_H_INCLUDED

#include "HCUBE_Experiment.h"
#include "HCUBE_AtariExperiment.h"
#include "HCUBE_AtariPixelExperiment.h"
#include "ale_interface.hpp"
#include "common/visual_processor.h"

/**
   This class is meant to experiment with how well HyperNEAT can do
   when running off of pure noise.  The experiment itself is
   identical to the Pixel Experiment except that instead of giving it
   the game screen, we give it pure random static in the form of
   seeded random numbers.
*/
namespace HCUBE
{
    class AtariNoiseExperiment : public AtariExperiment
    {
    public: 
        AtariNoiseExperiment(string _experimentName, int _threadID);

        virtual void initializeExperiment(string rom_name);
        virtual void initializeTopology();
        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);

        // Sets the substrate values to pure static
        virtual void setSubstrateValues(NEAT::LayeredSubstrate<float>* substrate);
    };
}

#endif // HCUBE_ATARINOISEEXPERIMENT_H_INCLUDED
