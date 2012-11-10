#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "Experiments/HCUBE_AtariExperiment.h"
#include "Experiments/HCUBE_AtariNoGeomExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatExperiment.cpp"
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

  // Quit if we don't have I/O/G
  if (!commandLineParser.HasSwitch("-I") ||
      !commandLineParser.HasSwitch("-O") ||
      !commandLineParser.HasSwitch("-G")) {
    cout << "[HyperNEAT core] Syntax for passing command-line options to HyperNEAT (do not actually type '(' or ')' ):\n";
    cout << "[HyperNEAT core] ./atari_generate [-R (seed)] -I (datafile) -O (outputfile) -G (ROMFile) [-P (populationfile) -F (fitnessprefix) -E (evaluationFile)]\n";
    cout << "[HyperNEAT core] \t(datafile) experiment data file - typically data/AtariExperiment.dat\n";
    cout << "                 \t(outputfile) the next generation file containging all the individual in xml.gz format(also refers to initial file produced for generation 0) - typically generationXX.xml\n";
    cout << "                 \t(populationfile) the current generation file (required when outputfile is > generation0) - typically generationXX(-1).xml.gz\n";
    cout << "                 \t(fitnessprefix) used to locate the fitness files for individuals in the current generation (required for generation > 0) - typically fitness.XX.\n";
    cout << "                 \t(evaluationfile) populationfile + fitness + speciation (output only - not required for next cycle) - typicall generationXX(-1).eval.xml\n";
    return retval;
  }

  NEAT::Globals::init(commandLineParser.GetSafeArgument("-I",0,"input.dat"));

  // Has the user specified a random seed?
  if (commandLineParser.HasSwitch("-R")) {
      NEAT::Globals::getSingleton()->seedRandom(stringTo<unsigned int>(commandLineParser.GetSafeArgument("-R",0,"0")));
  }

  int experimentType = int(NEAT::Globals::getSingleton()->getParameterValue("ExperimentType") + 0.001);
  HCUBE::ExperimentRun experimentRun;

  // Is this an experiment in progress? If so we should load the current experiment
  if (commandLineParser.HasSwitch("-P") &&
      commandLineParser.HasSwitch("-F") &&
      commandLineParser.HasSwitch("-E")) {
      string populationFile = commandLineParser.GetSafeArgument("-P",0,"population.xml");
      string fitnessFunctionPrefix = commandLineParser.GetSafeArgument("-F",0,"fitness.0.");
      string evaluationFile = commandLineParser.GetSafeArgument("-E",0,"evaluation.xml");
      cout << "[HyperNEAT core] Population for existing generation created from: " << populationFile << endl;

      experimentRun.setupExperimentInProgress(populationFile,commandLineParser.GetSafeArgument("-O",0,"output.xml"));
      experimentRun.createPopulationFromCondorRun(populationFile, fitnessFunctionPrefix, evaluationFile);
  } else {
      cout << "[HyperNEAT core] Population for first generation created\n";
      experimentRun.setupExperiment(experimentType, commandLineParser.GetSafeArgument("-O",0,"output.xml"));
      string rom_file = commandLineParser.GetSafeArgument("-G",0,"../ale/roms/asterix.bin");
      if (experimentType == 30) {
          boost::shared_ptr<HCUBE::AtariExperiment> exp = boost::static_pointer_cast<HCUBE::AtariExperiment>(experimentRun.getExperiment());
          exp->initializeExperiment(rom_file.c_str());
      } else if (experimentType == 31) {
          boost::shared_ptr<HCUBE::AtariNoGeomExperiment> exp = boost::static_pointer_cast<HCUBE::AtariNoGeomExperiment>(experimentRun.getExperiment());
          exp->initializeExperiment(rom_file.c_str());
      } else if (experimentType == 32) {
          boost::shared_ptr<HCUBE::AtariFTNeatExperiment> exp = boost::static_pointer_cast<HCUBE::AtariFTNeatExperiment>(experimentRun.getExperiment());
          exp->initializeExperiment(rom_file.c_str());
      }          

      experimentRun.createPopulation();
  }
  experimentRun.setCleanup(true);
  experimentRun.startCondor();

  NEAT::Globals::deinit();
  
  return retval;
}

int main(int argc,char **argv) {
  HyperNEAT_main(argc,argv);
}
