#include "setup.h"
#include "simulator.h"

int main(int argc, char** argv)
{
	auto Config = new SimulationErrorLocator::Setup::Configurator(argc, argv);
	auto Sim    = new SimulationErrorLocator::Simulator(Config);

	Sim->run();

	delete Sim;
	delete Config;

	return 0;
}
