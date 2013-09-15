#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "Experiments/HCUBE_AtariExperiment.h"
#include "Experiments/HCUBE_AtariNoGeomExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatPixelExperiment.h"
#include "Experiments/HCUBE_AtariIntrinsicExperiment.h"
#include "Experiments/HCUBE_AtariCMAExperiment.h"
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

    // Quit if we don't have I/O/G
    if (!commandLineParser.HasSwitch("-I") ||
        !commandLineParser.HasSwitch("-O") ||
        !commandLineParser.HasSwitch("-G")) {
        cout << "./atari_generate [-R (seed)] -I (datafile) -O (outputfile) -G (ROMFile) [-P (populationfile) -F (fitnessprefix) [-E (evaluationFile)] ]\n";
        cout << "\t(datafile) experiment data file - typically data/AtariExperiment.dat\n";
        cout << "\t(outputfile) the next generation file to be created - typically generationXX.xml\n";
        cout << "\t(populationfile) the current generation file (required when outputfile is > generation0) - typically generationXX(-1).xml.gz\n";
        cout << "\t(fitnessprefix) used to locate the fitness files for individuals in the current generation (required for generation > 0) - typically fitness.XX.\n";
        cout << "\t(evaluationfile) populationfile + fitness + speciation (output only - not required for next cycle) - typicall generationXX(-1).eval.xml\n";
        return 0;
    }

    Globals *globals = Globals::init(commandLineParser.GetArgument("-I",0));

    if (commandLineParser.HasSwitch("-R")) {
        unsigned int seed = stringTo<unsigned int>(commandLineParser.GetArgument("-R",0));
        globals->setParameterValue("RandomSeed",double(seed));
        globals->initRandom();
    }

    // Setup the experiment
    int experimentType = int(globals->getParameterValue("ExperimentType") + 0.001);
    HCUBE::ExperimentRun experimentRun;
    string out_file = commandLineParser.GetArgument("-O",0);
    experimentRun.setupExperiment(experimentType, out_file);

    string rom_file = commandLineParser.GetArgument("-G",0);

    // Is this an experiment in progress? If so we should load the current experiment
    if (commandLineParser.HasSwitch("-P") &&
        commandLineParser.HasSwitch("-F")) {
        string populationFile = commandLineParser.GetArgument("-P",0);
        string fitnessFunctionPrefix = commandLineParser.GetArgument("-F",0);
        string evaluationFile = commandLineParser.GetSafeArgument("-E",0,"");
        cout << "[HyperNEAT core] Population for existing generation created from: " << populationFile << endl;
        experimentRun.createPopulationFromCondorRun(populationFile, fitnessFunctionPrefix, evaluationFile, rom_file);
    } else {
        cout << "[HyperNEAT core] Population for first generation created" << endl;
        shared_ptr<Experiment> e = experimentRun.getExperiment();        
        if (experimentType == 30 || experimentType == 35 || experimentType == 36) {
            shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 31 || experimentType == 39 || experimentType == 40) {
            shared_ptr<AtariNoGeomExperiment> exp = static_pointer_cast<AtariNoGeomExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 32 || experimentType == 37 || experimentType == 38) {
            shared_ptr<AtariFTNeatExperiment> exp = static_pointer_cast<AtariFTNeatExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 33) {
            // This is the Hybrid experiment but always starts as HyperNEAT
            shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 34) {
            shared_ptr<AtariIntrinsicExperiment> exp = static_pointer_cast<AtariIntrinsicExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 41) {
            shared_ptr<AtariCMAExperiment> exp = static_pointer_cast<AtariCMAExperiment>(e);
            exp->setResultsPath(out_file);
            exp->initializeExperiment(rom_file.c_str());
        }
        experimentRun.createPopulation();
    }
    experimentRun.startCondor();

    NEAT::Globals::deinit();
}

int main(int argc,char **argv) {
    HyperNEAT_main(argc,argv);
}
