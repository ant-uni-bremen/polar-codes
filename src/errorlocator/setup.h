/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCERL_SETUP_H
#define PCERL_SETUP_H

#include <map>
#include <string>

#include <tclap/CmdLine.h>

namespace SimulationErrorLocator {

namespace Setup {


/*!
 * \brief The Configurator class
 */
class Configurator {
	TCLAP::CmdLine *cmd;

	std::map<std::string, float> defaultFloats;
	std::map<std::string, int> defaultInts;
	std::map<std::string, long> defaultLongInts;
	std::map<std::string, std::string> defaultStrings;
	std::map<std::string, bool> defaultBools;

	std::map<std::string, TCLAP::Arg*> argumentList;

	void setupArgumentDefaults();
	void setupCommandlineArguments(TCLAP::CmdLine *cmd);

	void cleanupCommandlineArguments();
	void insertArgument(TCLAP::Arg* arg);

	void setupArgumentWorkload();
	void setupArgumentSnr();
	void setupArgumentDesignSnr();
	void setupArgumentBlockLength();
	void setupArgumentCoderate();
	void setupArgumentOutputFile();
	void setupArgumentThreadCount();

public:
	/*!
	 * \brief Create and initialize the configurator object using command line options.
	 * \param argc Number of arguments, given by OS.
	 * \param argv List of arguments, given by OS.
	 */
	Configurator(int argc, char **argv);
	~Configurator();

	/*!
	 * \brief Get the value of an argument of string type.
	 * \param name Name of the argument.
	 * \return Either the default value for the argument or the value given via command-line.
	 */
	std::string getString(std::string name);

	/*!
	 * \brief Get the value of an argument of integer type.
	 * \param name Name of the argument.
	 * \return Either the default value for the argument or the value given via command-line.
	 */
	int getInt(std::string name);

	/*!
	 * \brief Get the value of an argument of long integer type.
	 * \param name Name of the argument.
	 * \return Either the default value for the argument or the value given via command-line.
	 */
	long getLongInt(std::string name);

	/*!
	 * \brief Get the value of an argument of floating-point type.
	 * \param name Name of the argument.
	 * \return Either the default value for the argument or the value given via command-line.
	 */
	float getFloat(std::string name);

	/*!
	 * \brief Get the value of an argument switch.
	 * \param name Name of the argument.
	 * \return Either the default value "false", or true if the switch has been set via command-line.
	 */
	bool getSwitch(std::string name);

};


}// namespace Setup

}// namespace SimulationErrorLocator

#endif //PCERL_SETUP_H
