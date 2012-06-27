#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "HCUBE_ExperimentRun.h"

#include "cakepp.h"
#include "SgInit.h"
#include "GoInit.h"

#ifndef HCUBE_NOGUI
namespace HCUBE
{
  IMPLEMENT_APP_NO_MAIN(MainApp)
}
#endif

#define EPOCHS_PER_PRINT (100000)

int HyperNEAT_main(int argc,char **argv) {

  char str[1024];
  initcake(str);
  SgInit();
  GoInit();

  int retval = 0;
 
  CommandLineParser commandLineParser(argc,argv);

  if (commandLineParser.HasSwitch("-I") &&
      commandLineParser.HasSwitch("-O") &&
      commandLineParser.HasSwitch("-G")) {

    NEAT::Globals::init(commandLineParser.GetSafeArgument("-I",0,"input.dat"));
    if (commandLineParser.HasSwitch("-R")) {
      NEAT::Globals::getSingleton()->seedRandom(stringTo<unsigned int>(commandLineParser.GetSafeArgument("-R",0,"0")));
    }

    int experimentType = int(NEAT::Globals::getSingleton()->getParameterValue("ExperimentType") + 0.001);

    cout << "Loading Experiment: " << experimentType << endl;
    HCUBE::ExperimentRun experimentRun;
    experimentRun.setupExperiment(experimentType, commandLineParser.GetSafeArgument("-O",0,"output.xml"));

    string rom_file = commandLineParser.GetSafeArgument("-G",0,"../ale_v0.1/roms/asterix.bin");
    boost::shared_ptr<HCUBE::AtariExperiment> exp = boost::static_pointer_cast<HCUBE::AtariExperiment>(experimentRun.getExperiment());
    exp->setDisplayScreen(true);
    exp->set_rom(rom_file.c_str());

    cout << "Experiment set up\n";
    experimentRun.createPopulation();
    experimentRun.setCleanup(true);
    cout << "Population Created\n";
    experimentRun.start();

  } else {
    cout << "Syntax for passing command-line options to HyperNEAT (do not actually type '(' or ')' ):\n";
    cout << "./HyperNEAT [-R (seed)] -I (datafile) -O (outputfile) -G (ROMFile)\n";
  }

  NEAT::Globals::deinit();
  exitcake();
  GoFini();
  SgFini();
  
  return retval;
}

int main(int argc,char **argv) {
  HyperNEAT_main(argc,argv);
}
