#include "setup.h"

#include <iostream>

namespace Simulation {

namespace Setup {

using namespace std;
using namespace TCLAP;

/* The default values for simulation parameters might be put into an external
 * configuration file later. For the moment, collect them hard-coded in the
 * beginning of this file.
 */
void Configurator::setupArgumentDefaults() {
	defaultStrings.insert({"simtype", "single"});

	defaultLongInts.insert({"workload", 4e8L});

	defaultFloats.insert({"snr-min", -1.5});
	defaultFloats.insert({"snr-max",  6.0});
	defaultInts.insert({"snr-count", 12});

	defaultFloats.insert({"dsnr-fixed", 0.0});
	defaultFloats.insert({"dsnr-min",  -1.59});
	defaultFloats.insert({"dsnr-max",  10.0});
	defaultInts.insert({"dsnr-count",   6});

	defaultInts.insert({"n-fixed", 1024});
	defaultInts.insert({"n-min",    128});
	defaultInts.insert({"n-max",   4096});

	defaultFloats.insert({"r-fixed", 0.5});
	defaultFloats.insert({"r-min",   0.25});
	defaultFloats.insert({"r-max",   0.9});
	defaultInts.insert({"r-count",   5});

	defaultInts.insert({"l-fixed", 4});
	defaultInts.insert({"l-min",   1});
	defaultInts.insert({"l-max",  64});

	defaultStrings.insert({"errorDetection","crc32"});

	defaultBools.insert({"soft-output", false});
	defaultBools.insert({"non-systematic", false});

	defaultInts.insert({"precision", 832});

	defaultFloats.insert({"amp-fixed", 10.0});//found empirically
	defaultFloats.insert({"amp-min",   1.0});
	defaultFloats.insert({"amp-max", 128.0});
	defaultInts.insert({"amp-count",   6});

	defaultStrings.insert({"outputFile", "simulation"});

	defaultInts.insert({"threads", 1});
}


void Configurator::insertArgument(TCLAP::Arg* arg) {
	argumentList.insert({arg->getName(), arg});
}


void Configurator::setupArgumentSimType() {
	vector<string> SimTypesVector = {"single",
									 "codelength",
									 "designsnr",
									 "listlength",
									 "rate",
									 "amplification"};
	ValuesConstraint<string> *AvailableSimTypes = new ValuesConstraint<string>(SimTypesVector);

	auto SimType = new UnlabeledValueArg<string>(
							"simtype",
							"The simulation type specifies, which parameter should be varied.",
							false,//required
							defaultStrings["simtype"],
							AvailableSimTypes);
	insertArgument(SimType);
}

void Configurator::setupArgumentWorkload() {
	auto workload = new ValueArg<long>("w", "workload", "Set the number of bits per simulation run (default: 4e8)", false, defaultLongInts["workload"], "bits");
	insertArgument(workload);
}

void Configurator::setupArgumentSnr() {
	auto snrMin = new ValueArg<float>("", "snr-min", "Set the lower SNR (Eb/No) in dB.", false, defaultFloats["snr-min"], "float");
	auto snrMax = new ValueArg<float>("", "snr-max", "Set the upper SNR (Eb/No) in dB.", false, defaultFloats["snr-max"], "float");
	auto snrCount = new ValueArg<int>("", "snr-count", "Set the number of SNRs (and thus simulation runs per configuration).", false, defaultInts["snr-count"], "integer");
	insertArgument(snrMin);
	insertArgument(snrMax);
	insertArgument(snrCount);
}


void Configurator::setupArgumentDesignSnr() {
	auto dsnrFixed = new ValueArg<float>("d", "design-snr", "Set the fixed design-SNR parameter for Bhattacharrya code construction", false, defaultFloats["dsnr-fixed"], "float");
	auto dsnrMin = new ValueArg<float>("", "dsnr-min", "Set the lower design-SNR.", false, defaultFloats["dsnr-min"], "float");
	auto dsnrMax = new ValueArg<float>("", "dsnr-max", "Set the upper design-SNR.", false, defaultFloats["dsnr-max"], "float");
	auto dsnrCount = new ValueArg<int>("", "dsnr-count", "Set the number of design-SNRs.", false, defaultInts["dsnr-count"], "integer");
	insertArgument(dsnrFixed);
	insertArgument(dsnrMin);
	insertArgument(dsnrMax);
	insertArgument(dsnrCount);
}

void Configurator::setupArgumentBlockLength() {
	auto nFixed = new ValueArg<int>("n", "blocklength", "Length of a Polar Code block.", false, defaultInts["n-fixed"], "int");
	auto nMin = new ValueArg<int>("", "n-min", "Minimum blocklength.", false, defaultInts["n-min"], "int");
	auto nMax = new ValueArg<int>("", "n-max", "Maximum blocklength", false, defaultInts["n-max"], "int");
	insertArgument(nFixed);
	insertArgument(nMin);
	insertArgument(nMax);
}

void Configurator::setupArgumentCoderate() {
	auto rFixed = new ValueArg<float>("r", "rate", "Set the fixed code rate.", false, defaultFloats["r-fixed"], "float");
	auto rMin = new ValueArg<float>("", "r-min", "Set the lower code rate.", false, defaultFloats["r-min"], "float");
	auto rMax = new ValueArg<float>("", "r-max", "Set the upper code rate.", false, defaultFloats["r-max"], "float");
	auto rCount = new ValueArg<int>("", "r-count", "Set the number of code rates.", false, defaultInts["r-count"], "integer");
	insertArgument(rFixed);
	insertArgument(rMin);
	insertArgument(rMax);
	insertArgument(rCount);
}

void Configurator::setupArgumentListLength() {
	auto lFixed = new ValueArg<int>("l", "pathlimit", "For list decoding, set the maximum number of parallel paths.", false, defaultInts["l-fixed"], "int");
	auto lMin = new ValueArg<int>("", "l-min", "Minimum path limit", false, defaultInts["l-min"], "int");
	auto lMax = new ValueArg<int>("", "l-max", "Maximum path limit", false, defaultInts["l-max"], "int");
	insertArgument(lFixed);
	insertArgument(lMin);
	insertArgument(lMax);
}

void Configurator::setupArgumentErrorDetection() {
	vector<string> ErrDetTypesVector = {"none",
									 "crc8",
									 "crc32"};
	auto AvailableErrDets = new ValuesConstraint<string>(ErrDetTypesVector);

	auto ErrDet = new ValueArg<string>("e",
							"error-detection",
							"The error detection scheme to enhance list decoding.",
							false,//required
							defaultStrings["errorDetection"],
							AvailableErrDets);
	insertArgument(ErrDet);
}

void Configurator::setupSwitchArguments() {
	auto SoftOutput = new SwitchArg("f", "soft-output", "Enable soft-output decoding.", defaultBools["soft-output"]);
	auto Systematic = new SwitchArg("s", "non-systematic", "Disable systematic polar coding.", defaultBools["non-systematic"]);
	insertArgument(SoftOutput);
	insertArgument(Systematic);
}

void Configurator::setupArgumentDecodingPrecision() {
	auto Precision = new ValueArg<int>("p", "precision", "Select decoding precision (32-bit floating point, 8-bit fixed integer or '832' for mixed precision).", false, defaultInts["precision"], "8,32,832");
	insertArgument(Precision);
}

void Configurator::setupArgumentAmplification() {
	auto ampFixed = new ValueArg<float>("a", "amplification", "Set the fixed amplification factor for 8-bit pre-quantization scaling.", false, defaultFloats["amp-fixed"], "float");
	auto ampMin = new ValueArg<float>("", "amp-min", "Set the lower amplification factor.", false, defaultFloats["amp-min"], "float");
	auto ampMax = new ValueArg<float>("", "amp-max", "Set the upper amplification factor.", false, defaultFloats["amp-max"], "float");
	auto ampCount = new ValueArg<int>("", "amp-count", "Set the number of amplification factors.", false, defaultInts["amp-count"], "integer");
	insertArgument(ampFixed);
	insertArgument(ampMin);
	insertArgument(ampMax);
	insertArgument(ampCount);
}

void Configurator::setupArgumentOutputFile() {
	auto OutputFile = new ValueArg<string>("o",
										   "output",
										   "The basename of the output file. \"_[simtype].csv\" will be appended automatically.",
										   false,
										   defaultStrings["outputFile"],
										   "filename");
	insertArgument(OutputFile);
}

void Configurator::setupArgumentThreadCount() {
	auto ThreadCount = new ValueArg<int>("t", "threads", "Number of parallel simulation threads.", false, defaultInts["threads"], "int");
	insertArgument(ThreadCount);
}

void Configurator::setupCommandlineArguments(CmdLine *cmd) {
	setupArgumentDefaults();
	setupArgumentSimType();
	setupArgumentWorkload();
	setupArgumentSnr();
	setupArgumentDesignSnr();
	setupArgumentBlockLength();
	setupArgumentCoderate();
	setupArgumentListLength();
	setupArgumentErrorDetection();
	setupSwitchArguments();
	setupArgumentDecodingPrecision();
	setupArgumentAmplification();
	setupArgumentOutputFile();
	setupArgumentThreadCount();

	for(auto arg : argumentList) {
		cmd->add(arg.second);
	}
}

void Configurator::cleanupCommandlineArguments() {
	for(auto arg : argumentList) {
		delete arg.second;
	}
	argumentList.clear();
}


Configurator::Configurator(int argc, char **argv) {
	cmd = new CmdLine("Polar Coding Simulation");
	setupCommandlineArguments(cmd);
	cmd->parse(argc, argv);
}

Configurator::~Configurator() {
	cleanupCommandlineArguments();
	delete cmd;
}

string Configurator::getString(string name) {
	if(argumentList.find(name) == argumentList.end()) {
		cerr << "Configurator::getString: Cannot find argument \"" << name << "\"." << endl;
		return "";
	}
	return dynamic_cast<ValueArg<string>*>(argumentList[name])->getValue();
}

int Configurator::getInt(std::string name) {
	if(argumentList.find(name) == argumentList.end()) {
		cerr << "Configurator::getInt: Cannot find argument \"" << name << "\"." << endl;
		return 0;
	}
	return dynamic_cast<ValueArg<int>*>(argumentList[name])->getValue();
}

long Configurator::getLongInt(std::string name) {
	if(argumentList.find(name) == argumentList.end()) {
		cerr << "Configurator::getLongInt: Cannot find argument \"" << name << "\"." << endl;
		return 0;
	}
	return dynamic_cast<ValueArg<long>*>(argumentList[name])->getValue();
}

float Configurator::getFloat(std::string name) {
	if(argumentList.find(name) == argumentList.end()) {
		cerr << "Configurator::getFloat: Cannot find argument \"" << name << "\"." << endl;
		return 0.0;
	}
	return dynamic_cast<ValueArg<float>*>(argumentList[name])->getValue();
}

bool Configurator::getSwitch(std::string name) {
	if(argumentList.find(name) == argumentList.end()) {
		cerr << "Configurator::getSwitch: Cannot find argument \"" << name << "\"." << endl;
		return false;
	}
	return dynamic_cast<SwitchArg*>(argumentList[name])->getValue();
}


}//namespace Setup
}//namespace Simulation
