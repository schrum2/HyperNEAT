#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "Experiments/HCUBE_AtariExperiment.h"
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

#define EPOCHS_PER_PRINT (100000)

int HyperNEAT_main(int argc,char **argv) {

  int retval = 0;
 
  CommandLineParser commandLineParser(argc,argv);

// #ifndef HCUBE_NOGUI
//   cout << "Starting wxEntry...\n";
//   retval = wxEntry(argc, argv);
// #endif

  if (commandLineParser.HasSwitch("-I") &&  // Experiment params
      commandLineParser.HasSwitch("-P") &&  // Population file
      commandLineParser.HasSwitch("-N") &&  // Individual number
      commandLineParser.HasSwitch("-G"))    // Rom file
    {

    NEAT::Globals::init(commandLineParser.GetArgument("-I",0));
    if (commandLineParser.HasSwitch("-R")) {
      uint seed = stringTo<unsigned int>(commandLineParser.GetArgument("-R",0));
      NEAT::Globals::getSingleton()->setParameterValue("RandomSeed",double(seed));
    }

    int experimentType = int(NEAT::Globals::getSingleton()->getParameterValue("ExperimentType") + 0.001);

    cout << "[HyperNEAT core] Loading Experiment: " << experimentType << endl;
    HCUBE::ExperimentRun experimentRun;
    experimentRun.setupExperiment(experimentType, "output.xml");

    cout << "[HyperNEAT core] Population Created\n";
    string populationFile = commandLineParser.GetArgument("-P",0);
    experimentRun.createPopulation(populationFile);
    unsigned int individualId = stringTo<unsigned int>(commandLineParser.GetArgument("-N",0));

    cout << "[HyperNEAT core] Visualizing individual: " << individualId << endl;
    string rom_file = commandLineParser.GetArgument("-G",0);
    if (experimentType == 30) {
        boost::shared_ptr<HCUBE::AtariExperiment> exp = boost::static_pointer_cast<HCUBE::AtariExperiment>(experimentRun.getExperiment());
        exp->setDisplayScreen(true);
        exp->initializeExperiment(rom_file.c_str());
    } else if (experimentType == 31) {
        boost::shared_ptr<HCUBE::AtariNoGeomExperiment> exp = boost::static_pointer_cast<HCUBE::AtariNoGeomExperiment>(experimentRun.getExperiment());
        exp->setDisplayScreen(true);
        exp->initializeExperiment(rom_file.c_str());
    } else if (experimentType == 32) {
        boost::shared_ptr<HCUBE::AtariFTNeatExperiment> exp = boost::static_pointer_cast<HCUBE::AtariFTNeatExperiment>(experimentRun.getExperiment());
        exp->setDisplayScreen(true);
        exp->initializeExperiment(rom_file.c_str());
    } else if (experimentType == 33) {
        boost::shared_ptr<HCUBE::AtariIntrinsicExperiment> exp = boost::static_pointer_cast<HCUBE::AtariIntrinsicExperiment>(experimentRun.getExperiment());
        exp->setDisplayScreen(true);
        exp->initializeExperiment(rom_file.c_str());
    }

    float fitness = experimentRun.evaluateIndividual(individualId);
    cout << "[HyperNEAT core] Fitness found to be " << fitness << endl;

  } else {
    cout << "Syntax for passing command-line options to HyperNEAT (do not actually type '(' or ')' ):\n";
    cout << "./atari_visualize [-R (seed)] -I (datafile) -P (populationfile) -N (individualId) -G (ROMfile)\n";
    cout << "\t\t(datafile) HyperNEAT experiment data file - typically data/AtariExperiment.dat\n";
    cout << "\t\t(populationfile) current population file containing all the individuals - typically generationXX.xml.gz\n";
    cout << "\t\t(individualId) unsigned int specifying which particular individual from the above population file we are evaluating\n";
    cout << "\t\t(ROMfile) the atari games file which should be loaded\n";
  }

  NEAT::Globals::deinit();
  
  return retval;
}

int main(int argc,char **argv) {
  HyperNEAT_main(argc,argv);
}
