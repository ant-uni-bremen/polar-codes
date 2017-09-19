#include <signalprocessing/random.h>

namespace SignalProcessing {
namespace Random {

#ifdef __RDRND__
void Generator::get(uint32_t *ptr) {
	_rdrand32_step(ptr);
}
void Generator::get64(uint64_t *ptr) {
	_rdrand64_step(reinterpret_cast<long long unsigned*>(ptr));
}
void Generator::get64x4(uint64_t *ptr) {
	long long unsigned *llptr = reinterpret_cast<long long unsigned*>(ptr);
	_rdrand64_step(llptr);
	_rdrand64_step(llptr+1);
	_rdrand64_step(llptr+2);
	_rdrand64_step(llptr+3);
}
#else
void Generator::get(uint32_t *ptr)
{
	mtx.lock();
	*ptr = static_cast<uint32_t>(generator());
	mtx.unlock();
}
void Generator::get64(uint64_t *ptr)
{
	mtx.lock();
	*ptr = generator();
	mtx.unlock();
}
void Generator::get64x4(uint64_t *ptr)
{
	mtx.lock();
	ptr[0] = generator();
	ptr[1] = generator();
	ptr[2] = generator();
	ptr[3] = generator();
	mtx.unlock();
}
#endif

}//namespace Random
}//namespace SignalProcessing
