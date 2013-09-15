#include "HCUBE_Defines.h"
#include "JGTL_CommandLineParser.h"
  
#ifndef HCUBE_NOGUI
# include "HCUBE_MainApp.h"
#endif

#include "HCUBE_ExperimentRun.h"
#include "Experiments/HCUBE_AtariExperiment.h"
#include "Experiments/HCUBE_AtariNoGeomExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatExperiment.h"
#include "Experiments/HCUBE_AtariFTNeatPixelExperiment.h"
#include "Experiments/HCUBE_AtariIntrinsicExperiment.h"
#include "Experiments/HCUBE_AtariCMAExperiment.h"

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
            unsigned int seed = stringTo<unsigned int>(commandLineParser.GetArgument("-R",0));
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
            // This is the Hybrid experiment and can thus be either HyperNEAT or FT-NEAT
            if (globals->hasParameterValue("HybridConversionFinished") &&
                globals->getParameterValue("HybridConversionFinished") == 1.0) {
                // Make the FT-NEAT experiment active
                experimentRun.setActiveExperiment(1);
                e = experimentRun.getExperiment();
                shared_ptr<AtariFTNeatExperiment> exp = static_pointer_cast<AtariFTNeatExperiment>(e);
                exp->initializeExperiment(rom_file.c_str());
            } else {
                // shared_ptr<AtariExperiment> exp = static_pointer_cast<AtariExperiment>(e);
                // exp->initializeExperiment(rom_file.c_str());

                // This is a nasty-hack like short circuit of the
                // normal evaluation procedure. It is used for
                // HyperNEAT evaluation in Hybrid experiments. The
                // crux of this method is to convert the hyperneat
                // indvidual to be evaluated into a FT-individual and
                // then perform the eval using FT methods. This should
                // ensure that when the swap is done, fitness does not
                // drop off as a result of using different types of
                // networks to evaluate individuals.
                // This is the early generational case before the swap has happened
                shared_ptr<AtariExperiment> atariExp = static_pointer_cast<AtariExperiment>(e);
                // Initialize both experiments
                atariExp->initializeExperiment(rom_file.c_str());
                experimentRun.setActiveExperiment(1);
                e = experimentRun.getExperiment();
                shared_ptr<AtariFTNeatExperiment> ftExp = static_pointer_cast<AtariFTNeatExperiment>(e);
                ftExp->initializeExperiment(rom_file.c_str());
                // Get the individual to evaluate
                shared_ptr<NEAT::GeneticPopulation> population = experimentRun.getPopulation();
                shared_ptr<NEAT::GeneticGeneration> generation = population->getGeneration();
                shared_ptr<NEAT::GeneticIndividual> HyperNEAT_individual = generation->getIndividual(individualId);
                atariExp->substrate.populateSubstrate(HyperNEAT_individual);
                NEAT::LayeredSubstrate<float>* HyperNEAT_substrate = &atariExp->substrate;
                GeneticPopulation* FTNEAT_population = ftExp->createInitialPopulation(1);
                shared_ptr<GeneticIndividual> FTNEAT_individual = FTNEAT_population->getGeneration()->getIndividual(0);
                ftExp->convertIndividual(FTNEAT_individual, HyperNEAT_substrate);
                ftExp->evaluateIndividual(FTNEAT_individual);
                float fitness = FTNEAT_individual->getFitness();
                string individualFitnessFile = commandLineParser.GetArgument("-F",0);
                cout << "[HyperNEAT core] Fitness found to be " << fitness << ". Writing to: " <<
                    individualFitnessFile << endl;
                ofstream fout(individualFitnessFile.c_str());
                fout << fitness << endl;
                fout.close();
                cout << "[HyperNEAT core] Individual evaluation fin." << endl;
                exit(0);
            }
        } else if (experimentType == 34) {
            shared_ptr<AtariIntrinsicExperiment> exp = static_pointer_cast<AtariIntrinsicExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
        } else if (experimentType == 41) {
            shared_ptr<AtariCMAExperiment> exp = static_pointer_cast<AtariCMAExperiment>(e);
            exp->initializeExperiment(rom_file.c_str());
            exp->individualToEvaluate = individualId;

            assert(commandLineParser.HasSwitch("-g"));
            unsigned int generationNum = stringTo<unsigned int>(commandLineParser.GetArgument("-g",0));
            cout << "Using generation number " << generationNum << endl;
            exp->generationNumber = generationNum;

            exp->setResultsPath(populationFile);
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
        cout << "./atari_evaluate [-R (seed) -g (generationNum)] -I (datafile) -P (populationfile) -N (individualId) "
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
