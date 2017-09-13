#ifndef PCSIM_SIMULATOR_H
#define PCSIM_SIMULATOR_H

#include "setup.h"

namespace Simulation {

/*!
 * \brief The Simulator class
 */
class Simulator {
	Setup::Configurator *mConfiguration;


public:
	/*!
	 * \brief Create a simulator object and configure it.
	 */
	Simulator(Setup::Configurator*);
	~Simulator();

	/*!
	 * \brief Run the simulation.
	 */
	void run();
};

}

#endif //PCSIM_SIMULATOR_H
