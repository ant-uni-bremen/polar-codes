#include <signalprocessing/modulation/modem.h>

namespace SignalProcessing {
namespace Modulation {

Modem::Modem()
	: mInputSignal(nullptr),
	  mOutputSignal(new std::vector<float>()),
	  mExternalInput(false) {
}

Modem::~Modem() {
	if(mInputSignal != nullptr && !mExternalInput) {
		delete mInputSignal;
	}
	delete mOutputSignal;
}

void Modem::setInputData(PolarCode::BitContainer *container) {
	if(mExternalInput || mInputSignal == nullptr) {
		mInputSignal = new std::vector<float>();
		mExternalInput = false;
	}

	mInputSignal->resize(container->size());
	container->getFloatBits(mInputSignal->data());
}

void Modem::setInputSignal(std::vector<float> *input) {
	if(!mExternalInput && mInputSignal != nullptr) {
		delete mInputSignal;
	}
	mInputSignal = input;
	mExternalInput = true;
}

std::vector<float>* Modem::outputSignal() {
	return mOutputSignal;
}

void Modem::getDataOutput(PolarCode::BitContainer *dstContainer) {
	dstContainer->setSize(mOutputSignal->size());
	return dstContainer->insertLlr(mOutputSignal->data());
}


}//namespace Modulation
}//namespace SignalProcessing
