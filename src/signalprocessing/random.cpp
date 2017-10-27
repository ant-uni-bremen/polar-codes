#include <signalprocessing/random.h>

namespace SignalProcessing {
namespace Random {

Generator::Generator() {
#ifndef __RDRND__
	{
		using namespace std::chrono;
		uint64_t uniseed;
		high_resolution_clock::time_point now = high_resolution_clock::now();
		high_resolution_clock::duration dn = now.time_since_epoch();
		uniseed = dn.count();
		Uniform.Generator.seed(uniseed);
	}
#endif

	union {
		__m256i seed256;
		uint64_t seed64[4];
	};

	get64x4(seed64);
	Normal.Generator.seed(seed256);
}

Generator::~Generator() {
}

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
	Uniform.Mutex.lock();
	*ptr = static_cast<uint32_t>(Uniform.Generator());
	Uniform.Mutex.unlock();
}
void Generator::get64(uint64_t *ptr)
{
	Uniform.Mutex.lock();
	*ptr = Uniform.Generator();
	Uniform.Mutex.unlock();
}
void Generator::get64x4(uint64_t *ptr)
{
	Uniform.Mutex.lock();
	ptr[0] = generator();
	ptr[1] = generator();
	ptr[2] = generator();
	ptr[3] = generator();
	Uniform.Mutex.unlock();
}
#endif

void Generator::getNormDist(__m256 *a, __m256 *b) {
	static const __m256 twopi = _mm256_set1_ps(2.0f * 3.14159265358979323846f);
	static const __m256 one = _mm256_set1_ps(1.0f);
	static const __m256 minustwo = _mm256_set1_ps(-2.0f);

	__m256 u1 = _mm256_sub_ps(one, Normal.Generator()); // [0, 1) -> (0, 1]
	__m256 u2 = Normal.Generator();
	__m256 radius = _mm256_sqrt_ps(_mm256_mul_ps(
										minustwo,
										log256_ps(u1)));
	__m256 theta = _mm256_mul_ps(twopi, u2);
	__m256 sintheta, costheta;
	sincos256_ps(theta, &sintheta, &costheta);
	*a = _mm256_mul_ps(radius, costheta);
	*b = _mm256_mul_ps(radius, sintheta);
}

void Generator::getRayleighDist(__m256 *a, __m256 *b) {
	__m256 norm[4];
	getNormDist(norm,   norm+1);
	getNormDist(norm+2, norm+3);

	for(int i=0; i<4; ++i)
		norm[i] = _mm256_mul_ps(norm[i], norm[i]);

	//return square root of (R²+I²)
	*a = _mm256_sqrt_ps(_mm256_add_ps(norm[0], norm[2]));
	*b = _mm256_sqrt_ps(_mm256_add_ps(norm[1], norm[3]));
}

}//namespace Random
}//namespace SignalProcessing
