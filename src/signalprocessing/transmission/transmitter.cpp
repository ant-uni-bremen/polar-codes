#include <signalprocessing/transmission/transmitter.h>

namespace SignalProcessing {
namespace Transmission {

Transmitter::Transmitter() {
}

Transmitter::~Transmitter() {
}

void Transmitter::setSignal(std::vector<float> *sig) {
	mSignal = sig;
}

}//namespace Transmission
}//namespace SignalProcessing
