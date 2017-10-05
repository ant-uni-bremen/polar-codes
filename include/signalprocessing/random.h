#ifndef PCDSP_RANDOM_H
#define PCDSP_RANDOM_H

#include "lcg.h"
#include <mutex>

namespace SignalProcessing {
namespace Random {

/*!
 * \brief Pseudo-random number generator
 */
struct Generator {
#ifndef __RDRND__
#warning RDRND not used
	LCG<uint64_t> generator;///< Linear congruential generator
	std::mutex mtx;///< Mutex for multi-threading compatability
#endif

	void get(uint32_t *ptr);///< Get an unsigned 32-bit integer
	void get64(uint64_t *ptr);///< Get an unsigned 64-bit integer
	void get64x4(uint64_t *ptr);///< Get four unsigned 64-bit integers
};


}//namespace Random
}//namespace SignalProcessing

#endif
