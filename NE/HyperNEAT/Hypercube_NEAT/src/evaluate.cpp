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

#define EPOCHS_PER_PRINT (100000)

int HyperNEAT_main(int argc,char **argv) {

  int retval = 0;
 
  CommandLineParser commandLineParser(argc,argv);

  if (commandLineParser.HasSwitch("-I") && // Experiment params
      commandLineParser.HasSwitch("-F") && // Fitness file to write to
      commandLineParser.HasSwitch("-P") && // Population file to read from
      commandLineParser.HasSwitch("-N") && // Individual number within pop file
      commandLineParser.HasSwitch("-G"))   // Rom file to run
      {

    NEAT::Globals::init(commandLineParser.GetArgument("-I",0));
    if (commandLineParser.HasSwitch("-R")) {
      uint seed = stringTo<unsigned int>(commandLineParser.GetArgument("-R",0));
      NEAT::Globals::getSingleton()->setParameterValue("RandomSeed",double(seed));
      NEAT::Globals::getSingleton()->initRandom();
    }

    int experimentType = int(NEAT::Globals::getSingleton()->getParameterValue("ExperimentType") + 0.001);

    cout << "[HyperNEAT core] Loading Experiment: " << experimentType << endl;
    HCUBE::ExperimentRun experimentRun;
    experimentRun.setupExperiment(experimentType, "output.xml");

    cout << "[HyperNEAT core] Population Created\n";
    string populationFile = commandLineParser.GetArgument("-P",0);
    experimentRun.createPopulation(populationFile);
    unsigned int individualId = stringTo<unsigned int>(commandLineParser.GetArgument("-N",0));

    cout << "[HyperNEAT core] Evaluating individual: " << individualId << endl;
    string rom_file = commandLineParser.GetArgument("-G",0);
    if (experimentType == 30) {
        boost::shared_ptr<HCUBE::AtariExperiment> exp = boost::static_pointer_cast<HCUBE::AtariExperiment>(experimentRun.getExperiment());
        exp->setDisplayScreen(false);
        exp->initializeExperiment(rom_file.c_str());
    } else if (experimentType == 31) {
        boost::shared_ptr<HCUBE::AtariNoGeomExperiment> exp = boost::static_pointer_cast<HCUBE::AtariNoGeomExperiment>(experimentRun.getExperiment());
        exp->initializeExperiment(rom_file.c_str());
    } else if (experimentType == 32) {
        boost::shared_ptr<HCUBE::AtariFTNeatExperiment> exp = boost::static_pointer_cast<HCUBE::AtariFTNeatExperiment>(experimentRun.getExperiment());
        exp->initializeExperiment(rom_file.c_str());
    } else if (experimentType == 33) {
        boost::shared_ptr<HCUBE::AtariIntrinsicExperiment> exp = boost::static_pointer_cast<HCUBE::AtariIntrinsicExperiment>(experimentRun.getExperiment());
        exp->initializeExperiment(rom_file.c_str());
    }

    float fitness = experimentRun.evaluateIndividual(individualId);

    string individualFitnessFile = 
        commandLineParser.GetArgument("-F",0);
    cout << "[HyperNEAT core] Fitness found to be " << fitness << ". Writing to: " << individualFitnessFile << endl;
    ofstream fout(individualFitnessFile.c_str());
    fout << fitness << endl;
    fout.close();
    cout << "[HyperNEAT core] Individual evaluation fin." << endl;

  } else {
    cout << "Syntax for passing command-line options to HyperNEAT (do not actually type '(' or ')' ):\n";
    cout << "./atari_evaluate [-R (seed)] -I (datafile) -P (populationfile) -N (individualId) -F (fitnessFile) -G (romFile)\n";
    cout << "\t\t(datafile) HyperNEAT experiment data file - typically data/AtariExperiment.dat\n";
    cout << "\t\t(populationfile) current population file containing all the individuals - typically generationXX.xml.gz\n";
    cout << "\t\t(individualId) unsigned int specifying which particular individual from the above population file we are evaluating\n";
    cout << "\t\t(fitnessFile) fitness value once estimated written to file - typically fitness.XX.individualId\n";
    cout << "\t\t(romFile) the Atari rom file to evaluate the agent against.\n";
  }

  NEAT::Globals::deinit();
  
  return retval;
}

int main(int argc,char **argv) {
  HyperNEAT_main(argc,argv);
}
