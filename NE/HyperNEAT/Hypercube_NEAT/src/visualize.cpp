#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "Experiments/HCUBE_AtariExperiment.h"
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

  if (commandLineParser.HasSwitch("-I") &&  // Experiment params
      commandLineParser.HasSwitch("-P") &&  // Population file
      commandLineParser.HasSwitch("-N") &&  // Individual number
      commandLineParser.HasSwitch("-G"))    // Rom file
    {

    NEAT::Globals::init(commandLineParser.GetSafeArgument("-I",0,"input.dat"));
    if (commandLineParser.HasSwitch("-R")) {
      NEAT::Globals::getSingleton()->seedRandom(stringTo<unsigned int>(commandLineParser.GetSafeArgument("-R",0,"0")));
    }

    int experimentType = int(NEAT::Globals::getSingleton()->getParameterValue("ExperimentType") + 0.001);

    cout << "[HyperNEAT core] Loading Experiment: " << experimentType << endl;
    HCUBE::ExperimentRun experimentRun;
    experimentRun.setupExperiment(experimentType, "output.xml");

    cout << "[HyperNEAT core] Population Created\n";
    string populationFile = commandLineParser.GetSafeArgument("-P",0,"population.xml");
    experimentRun.createPopulation(populationFile);
    unsigned int individualId = stringTo<unsigned int>(commandLineParser.GetSafeArgument("-N",0,"0"));

    cout << "[HyperNEAT core] Visualizing individual: " << individualId << endl;
    string rom_file = commandLineParser.GetSafeArgument("-G",0,"../ale_v0.1/roms/asterix.bin");
    boost::shared_ptr<HCUBE::AtariExperiment> exp = boost::static_pointer_cast<HCUBE::AtariExperiment>(experimentRun.getExperiment());
    exp->setDisplayScreen(true);
    exp->set_rom(rom_file.c_str());
    float fitness = experimentRun.evaluateIndividual(individualId);

    cout << "[HyperNEAT core] Fitness found to be " << fitness << endl;

  } else {
    cout << "Syntax for passing command-line options to HyperNEAT (do not actually type '(' or ')' ):\n";
    cout << "./atari_evaluate [-R (seed)] -I (datafile) -P (populationfile) -N (individualId) -F (fitnessFile)\n";
    cout << "\t\t(datafile) HyperNEAT experiment data file - typically data/AtariExperiment.dat\n";
    cout << "\t\t(populationfile) current population file containing all the individuals - typically generationXX.xml.gz\n";
    cout << "\t\t(individualId) unsigned int specifying which particular individual from the above population file we are evaluating\n";
  }

  NEAT::Globals::deinit();
  
  return retval;
}

int main(int argc,char **argv) {
  HyperNEAT_main(argc,argv);
}
