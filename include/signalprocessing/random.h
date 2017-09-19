#ifndef PCDSP_RANDOM_H
#define PCDSP_RANDOM_H

#include "lcg.h"
#include <mutex>

namespace SignalProcessing {
namespace Random {

struct Generator {
#ifndef __RDRND__
#warning RDRND not used
	LCG<uint64_t> generator;
	std::mutex mtx;
#endif

	void get(uint32_t *ptr);
	void get64(uint64_t *ptr);
	void get64x4(uint64_t *ptr);
};


}//namespace Random
}//namespace SignalProcessing

#endif
