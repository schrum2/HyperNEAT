#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "Experiments/HCUBE_AtariExperiment.h"
#include "Experiments/HCUBE_AtariPixelExperiment.h" // Schrum: Needed to set number of processing layers
#include "Experiments/HCUBE_AtariNoGeomExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatExperiment.h"
#include "Experiments/HCUBE_AtariIntrinsicExperiment.h"
#include "HCUBE_ExperimentRun.h"

#ifndef HCUBE_NOGUI
namespace HCUBE
{
    IMPLEMENT_APP_NO_MAIN(MainApp)
}
#endif

using namespace boost;
using namespace HCUBE;
using namespace NEAT;

int HyperNEAT_main(int argc,char **argv) {
    CommandLineParser commandLineParser(argc,argv);

    if (commandLineParser.HasSwitch("-I") &&  // Experiment params
        commandLineParser.HasSwitch("-P") &&  // Population file
        commandLineParser.HasSwitch("-N") &&  // Individual number
        commandLineParser.HasSwitch("-G"))    // Rom file
    {

        Globals *globals = Globals::init(commandLineParser.GetArgument("-I",0));
        int experimentType = int(globals->getParameterValue("ExperimentType") + 0.001);
        cout << "[HyperNEAT core] Loading Experiment: " << experimentType << endl;
        HCUBE::ExperimentRun experimentRun;
        experimentRun.setupExperiment(experimentType, "output.xml");

        cout << "[HyperNEAT core] Population Created\n";
        string populationFile = commandLineParser.GetArgument("-P",0);
        experimentRun.createPopulation(populationFile);
        unsigned int individualId = stringTo<unsigned int>(commandLineParser.GetArgument("-N",0));

        if (commandLineParser.HasSwitch("-R")) {
            double seed = stringTo<double>(commandLineParser.GetArgument("-R",0));
            globals->setParameterValue("RandomSeed",seed);
            globals->initRandom();
        }

        cout << "[HyperNEAT core] Visualizing individual: " << individualId << endl;
        string rom_file = commandLineParser.GetArgument("-G",0);
        shared_ptr<Experiment> e = experimentRun.getExperiment();

	// Schrum: Allow variable number of processing layers
	if (experimentType == 35) {
	    // cout << "visualize: MY CODE" << endl;
            shared_ptr<AtariPixelExperiment> exp = static_pointer_cast<AtariPixelExperiment>(e);
            int numProcessingLayers = int(globals->getParameterValue("ProcessingLayers") + 0.001);
	    // Schrum: Want to allow for more flexability in substrate organization
	    exp->setProcessingLayers(numProcessingLayers);	
            cout << "[HyperNEAT core] Number of processing layers is: " << numProcessingLayers << endl;
            exp->initializeExperiment(rom_file.c_str());
	} else if (experimentType == 30 || experimentType == 36) {
            shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
            exp->setDisplayScreen(true);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 31) {
            shared_ptr<AtariNoGeomExperiment> exp = static_pointer_cast<AtariNoGeomExperiment>(e);
            exp->setDisplayScreen(true);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 32) {
            shared_ptr<AtariFTNeatExperiment> exp = static_pointer_cast<AtariFTNeatExperiment>(e);
            exp->setDisplayScreen(true);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 33) {
            // This is the Hybrid experiment and can thus be either HyperNEAT or FT-NEAT
            if (globals->hasParameterValue("HybridConversionFinished") &&
                globals->getParameterValue("HybridConversionFinished") == 1.0) {
                // Make the FT-NEAT experiment active
                experimentRun.setActiveExperiment(1);
                e = experimentRun.getExperiment();
                shared_ptr<AtariFTNeatExperiment> exp = static_pointer_cast<AtariFTNeatExperiment>(e);
                exp->setDisplayScreen(true);
                exp->initializeExperiment(rom_file.c_str());
            } else {
                shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
                exp->setDisplayScreen(true);
                exp->initializeExperiment(rom_file.c_str());
            }
        }  else if (experimentType == 34) {
            shared_ptr<AtariIntrinsicExperiment> exp = static_pointer_cast<AtariIntrinsicExperiment>(e);
            exp->setDisplayScreen(true);
            exp->initializeExperiment(rom_file.c_str());
        }

        float fitness = experimentRun.evaluateIndividual(individualId);
        cout << "[HyperNEAT core] Fitness found to be " << fitness << endl;

    } else {
        cout << "Syntax for passing command-line options to HyperNEAT (do not actually type '(' or ')' ):\n";
        cout << "./atari_visualize [-R (seed)] -I (datafile) -P (populationfile) -N (individualId) -G (ROMfile)\n";
        cout << "\t(datafile) HyperNEAT experiment data file - typically data/AtariExperiment.dat\n";
        cout << "\t(populationfile) current population file containing all the individuals - typically generationXX.xml.gz\n";
        cout << "\t(individualId) unsigned int specifying which particular individual from the above population file we are evaluating\n";
        cout << "\t(ROMfile) the atari games file which should be loaded\n";
    }

    NEAT::Globals::deinit();
}

int main(int argc,char **argv) {
    HyperNEAT_main(argc,argv);
}
