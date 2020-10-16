/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "setup.h"

#include <iostream>

namespace SimulationErrorLocator {

namespace Setup {

using namespace std;
using namespace TCLAP;

/* The default values for simulation parameters might be put into an external
 * configuration file later. For the moment, collect them hard-coded in the
 * beginning of this file.
 */
void Configurator::setupArgumentDefaults()
{
    defaultLongInts.insert({ "workload", 10000 });
    defaultFloats.insert({ "snr", 2.0 });
    defaultFloats.insert({ "dsnr", 0.0 });
    defaultInts.insert({ "n", 1024 });
    defaultFloats.insert({ "r", 0.5 });
    defaultBools.insert({ "non-systematic", true });
    defaultStrings.insert({ "outputFile", "badbits" });
    defaultInts.insert({ "threads", 1 });
}


void Configurator::insertArgument(TCLAP::Arg* arg)
{
    argumentList.insert({ arg->getName(), arg });
}


void Configurator::setupArgumentWorkload()
{
    auto workload = new ValueArg<long>("w",
                                       "workload",
                                       "Set the number of blocks to examine",
                                       false,
                                       defaultLongInts["workload"],
                                       "bits");
    insertArgument(workload);
}

void Configurator::setupArgumentSnr()
{
    auto snr = new ValueArg<float>(
        "s", "snr", "Set the SNR (Eb/No) in dB.", false, defaultFloats["snr"], "float");
    insertArgument(snr);
}


void Configurator::setupArgumentDesignSnr()
{
    auto dsnrFixed = new ValueArg<float>(
        "d",
        "design-snr",
        "Set the design-SNR parameter for Bhattacharrya code construction",
        false,
        defaultFloats["dsnr"],
        "float");
    insertArgument(dsnrFixed);
}

void Configurator::setupArgumentBlockLength()
{
    auto nFixed = new ValueArg<int>("n",
                                    "blocklength",
                                    "Length of a Polar Code block.",
                                    false,
                                    defaultInts["n"],
                                    "int");
    insertArgument(nFixed);
}

void Configurator::setupArgumentCoderate()
{
    auto rFixed = new ValueArg<float>(
        "r", "rate", "Set the code rate.", false, defaultFloats["r"], "float");
    insertArgument(rFixed);
}

void Configurator::setupArgumentOutputFile()
{
    auto OutputFile = new ValueArg<string>(
        "o",
        "output",
        "The basename of the output file. \".csv\" will be appended automatically.",
        false,
        defaultStrings["outputFile"],
        "filename");
    insertArgument(OutputFile);
}

void Configurator::setupArgumentThreadCount()
{
    auto ThreadCount = new ValueArg<int>("t",
                                         "threads",
                                         "Number of parallel simulation threads.",
                                         false,
                                         defaultInts["threads"],
                                         "int");
    insertArgument(ThreadCount);
}

void Configurator::setupCommandlineArguments(CmdLine* cmd)
{
    setupArgumentDefaults();
    setupArgumentWorkload();
    setupArgumentSnr();
    setupArgumentDesignSnr();
    setupArgumentBlockLength();
    setupArgumentCoderate();
    setupArgumentOutputFile();
    setupArgumentThreadCount();

    for (auto arg : argumentList) {
        cmd->add(arg.second);
    }
}

void Configurator::cleanupCommandlineArguments()
{
    for (auto arg : argumentList) {
        delete arg.second;
    }
    argumentList.clear();
}


Configurator::Configurator(int argc, char** argv)
{
    cmd = new CmdLine("Polar Coding Simulation - Bad bit finder");
    setupCommandlineArguments(cmd);
    cmd->parse(argc, argv);
}

Configurator::~Configurator()
{
    cleanupCommandlineArguments();
    delete cmd;
}

string Configurator::getString(string name)
{
    if (argumentList.find(name) == argumentList.end()) {
        cerr << "Configurator::getString: Cannot find argument \"" << name << "\"."
             << endl;
        return "";
    }
    return dynamic_cast<ValueArg<string>*>(argumentList[name])->getValue();
}

int Configurator::getInt(std::string name)
{
    if (argumentList.find(name) == argumentList.end()) {
        cerr << "Configurator::getInt: Cannot find argument \"" << name << "\"." << endl;
        return 0;
    }
    return dynamic_cast<ValueArg<int>*>(argumentList[name])->getValue();
}

long Configurator::getLongInt(std::string name)
{
    if (argumentList.find(name) == argumentList.end()) {
        cerr << "Configurator::getLongInt: Cannot find argument \"" << name << "\"."
             << endl;
        return 0;
    }
    return dynamic_cast<ValueArg<long>*>(argumentList[name])->getValue();
}

float Configurator::getFloat(std::string name)
{
    if (argumentList.find(name) == argumentList.end()) {
        cerr << "Configurator::getFloat: Cannot find argument \"" << name << "\"."
             << endl;
        return 0.0;
    }
    return dynamic_cast<ValueArg<float>*>(argumentList[name])->getValue();
}

bool Configurator::getSwitch(std::string name)
{
    if (argumentList.find(name) == argumentList.end()) {
        cerr << "Configurator::getSwitch: Cannot find argument \"" << name << "\"."
             << endl;
        return false;
    }
    return dynamic_cast<SwitchArg*>(argumentList[name])->getValue();
}


} // namespace Setup
} // namespace SimulationErrorLocator
