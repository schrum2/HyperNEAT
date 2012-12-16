#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "HCUBE_ExperimentRun.h"
#include "Experiments/HCUBE_AtariExperiment.h"
#include "Experiments/HCUBE_AtariNoGeomExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatExperiment.h"
#include "Experiments/HCUBE_AtariIntrinsicExperiment.h"

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
    Globals* globals = Globals::init();

    if (commandLineParser.HasSwitch("-I") && // Experiment params
        commandLineParser.HasSwitch("-F") && // Fitness file to write to
        commandLineParser.HasSwitch("-P") && // Population file to read from
        commandLineParser.HasSwitch("-N") && // Individual number within pop file
        commandLineParser.HasSwitch("-G"))   // Rom file to run
    {

        globals = Globals::init(commandLineParser.GetArgument("-I",0));
        if (commandLineParser.HasSwitch("-R")) {
            uint seed = stringTo<unsigned int>(commandLineParser.GetArgument("-R",0));
            globals->setParameterValue("RandomSeed",double(seed));
            globals->initRandom();
        }

        int experimentType = int(globals->getParameterValue("ExperimentType") + 0.001);

        cout << "[HyperNEAT core] Loading Experiment: " << experimentType << endl;
        HCUBE::ExperimentRun experimentRun;
        experimentRun.setupExperiment(experimentType, "output.xml");

        string populationFile = commandLineParser.GetArgument("-P",0);
        experimentRun.createPopulation(populationFile);
        cout << "[HyperNEAT core] Population Created\n";

        unsigned int individualId = stringTo<unsigned int>(commandLineParser.GetArgument("-N",0));
        cout << "[HyperNEAT core] Evaluating individual: " << individualId << endl;

        // Cast the experiment into the correct subclass and initialize with rom file
        string rom_file = commandLineParser.GetArgument("-G",0);
        shared_ptr<Experiment> e = experimentRun.getExperiment();
        if (experimentType == 30) {
            shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 31) {
            shared_ptr<AtariNoGeomExperiment> exp = static_pointer_cast<AtariNoGeomExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 32) {
            shared_ptr<AtariFTNeatExperiment> exp = static_pointer_cast<AtariFTNeatExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 33) {
            // This is the Hybrid experiment and can thus be either HyperNEAT or FT-NEAT
            if (globals->hasParameterValue("HybridConversionFinished") &&
                globals->getParameterValue("HybridConversionFinished") == 1.0) {
                // Make the FT-NEAT experiment active
                experimentRun.setActiveExperiment(1);
                e = experimentRun.getExperiment();
                shared_ptr<AtariFTNeatExperiment> exp = static_pointer_cast<AtariFTNeatExperiment>(e);
                exp->initializeExperiment(rom_file.c_str());
            } else {
                shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
                exp->initializeExperiment(rom_file.c_str());
            }
        }  else if (experimentType == 34) {
            shared_ptr<AtariIntrinsicExperiment> exp = static_pointer_cast<AtariIntrinsicExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        }

        float fitness = experimentRun.evaluateIndividual(individualId);

        string individualFitnessFile = 
            commandLineParser.GetArgument("-F",0);
        cout << "[HyperNEAT core] Fitness found to be " << fitness << ". Writing to: " <<
            individualFitnessFile << endl;
        ofstream fout(individualFitnessFile.c_str());
        fout << fitness << endl;
        fout.close();
        cout << "[HyperNEAT core] Individual evaluation fin." << endl;

    } else {
        cout << "./atari_evaluate [-R (seed)] -I (datafile) -P (populationfile) -N (individualId) "
            "-F (fitnessFile) -G (romFile)\n";
        cout << "\t\t(datafile) HyperNEAT experiment data file - typically data/AtariExperiment.dat\n";
        cout << "\t\t(populationfile) current population file containing all the individuals - "
            "typically generationXX.xml.gz\n";
        cout << "\t\t(individualId) unsigned int specifying which particular individual from the above"
            "population file we are evaluating\n";
        cout << "\t\t(fitnessFile) fitness value once estimated written to file - "
            "typically fitness.XX.individualId\n";
        cout << "\t\t(romFile) the Atari rom file to evaluate the agent against.\n";
    }

    globals->deinit();
}

int main(int argc,char **argv) {
    HyperNEAT_main(argc,argv);
}
