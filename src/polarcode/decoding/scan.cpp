#include <polarcode/decoding/scan.h>
#include <polarcode/decoding/templatized_float.h>
#include <cmath>
//#define SCAN_DEBUG_OUTPUT

namespace PolarCode {
namespace Decoding {

Scan::Scan(size_t blockLength, unsigned iterationLimit, const std::vector<unsigned> &frozenBits) {
	initialize(blockLength, iterationLimit, frozenBits);
}

Scan::~Scan() {

}

void Scan::setIterationLimit(unsigned iterationLimit) {
	mIterationLimit = iterationLimit;
}

void Scan::initialize(size_t blockLength, unsigned iterationLimit, const std::vector<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mIterationLimit = iterationLimit;

	mN = log2(blockLength);
	mLevelCount = mN + 1;

	{
		mBooleanFrozen.resize(mBlockLength);
		unsigned idx = 0;
		for(unsigned i = 0; i < mBlockLength; ++i) {
			if(i == mFrozenBits[idx]) {
				mBooleanFrozen[i] = true;
				++idx;
			} else {
				mBooleanFrozen[i] = false;
			}
		}
	}

	mLlrContainer = new FloatContainer(mBlockLength, mFrozenBits);
	mBitContainer = new FloatContainer(mBlockLength, mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength + 7) / 8];

	mLlr.resize(2 * mBlockLength - 1);
	mEven.resize(2 * mBlockLength - 1);
	mOdd.resize(mLevelCount * mBlockLength / 2);
}

static inline unsigned evenIndex(unsigned n, unsigned level, unsigned bit) {
	return bit + (1 << (n + 1)) - (1 << (n + 1 - level));
}

static inline unsigned oddIndex(unsigned n, unsigned level, unsigned group, unsigned bit) {
	if(level == n) {
		// Left shift by -1 is invalid. Instead, do the correct right shift.
		return bit + (group >> 1) + ((level - 1) << (n - 1));
	} else {
		return bit + ((group - 1) << (n - 1 - level)) + ((level - 1) << (n - 1));
	}
}

void Scan::updatellrmap(unsigned level, unsigned group) {
	if(level == 0) return;
	unsigned leftGroup = group / 2;
	if((group & 1) == 0) {
		updatellrmap(level - 1, leftGroup);
	}
	unsigned groupSize = 1 << (mN - level);

	if(group & 1) {
		for(unsigned node = 0; node < groupSize; ++node) {
			mLlr[evenIndex(mN, level, node)] =
					mLlr[evenIndex(mN, level - 1, 2 * node + 1)] +
					TemplatizedFloatCalc::F_function_calc(
						mLlr[evenIndex(mN, level - 1, 2 * node)],
						mEven[evenIndex(mN, level, node)]
						);
#ifdef SCAN_DEBUG_OUTPUT
			std::cout << "L(" << level << "," << node << ") = "
					  << "L(" << (level-1) << "," << (2*node+1) << ") + "
					  << "L(" << (level-1) << "," << (2*node) << ") BP "
					  << "E(" << level << "," << node << "): "
					  << mLlr[evenIndex(mN, level, node)] << " = "
					  << mLlr[evenIndex(mN, level - 1, 2 * node + 1)] << " + "
					  << mLlr[evenIndex(mN, level - 1, 2 * node)] << " BP "
					  << mEven[evenIndex(mN, level, node)]
					  << std::endl;
#endif
		}
	} else {
		for(unsigned node = 0; node < groupSize; ++node) {
			float t = mLlr[evenIndex(mN, level - 1, 2 * node + 1)]
					+ mOdd[oddIndex(mN, level, group + 1, node)];
			mLlr[evenIndex(mN, level, node)] =
					TemplatizedFloatCalc::F_function_calc(
						mLlr[evenIndex(mN, level - 1, 2 * node)],
						t);
#ifdef SCAN_DEBUG_OUTPUT
			std::cout << "L(" << level << "," << node << ") = "
					  << "L(" << (level-1) << "," << (2*node) << ") BP ["
					  << "L(" << (level-1) << "," << (2*node+1) << ") + "
					  << "O(" << level << "," << (group+1) << "," << node << ")]: "
					  << mLlr[evenIndex(mN, level, node)] << " = "
					  << mLlr[evenIndex(mN, level - 1, 2 * node)] << " BP ["
					  << mLlr[evenIndex(mN, level - 1, 2 * node + 1)] << " + "
					  << mOdd[oddIndex(mN, level, group + 1, node)] << "]"
					  << std::endl;
#endif
		}
	}
}

void Scan::updatebitmap(unsigned level, unsigned group) {
	if(group & 1) {
		unsigned leftGroup = group / 2;
		unsigned groupSize = 1 << (mN - level);
		if(leftGroup & 1) {
			for(unsigned node = 0; node < groupSize; ++node) {
				float t = mOdd[oddIndex(mN, level, group, node)]
						+ mLlr[evenIndex(mN, level - 1, 2 * node + 1)];
				mOdd[oddIndex(mN, level - 1, leftGroup, 2 * node)] =
						TemplatizedFloatCalc::F_function_calc(
							mEven[evenIndex(mN, level, node)],
							t);
#ifdef SCAN_DEBUG_OUTPUT
				std::cout << "O(" << (level-1) << "," << leftGroup << "," << (2*node) << ") = "
						  << "E(" << level << "," << node << ") BP ["
						  << "O(" << level << "," << group << "," << node << ") + "
						  << "L(" << (level-1) << "," << (2*node+1) << ")]: "
						  << mOdd[oddIndex(mN, level - 1, leftGroup, 2 * node)] << " = "
						  << mEven[evenIndex(mN, level, node)] << " BP ["
						  << mOdd[oddIndex(mN, level, group, node)] << " + "
						  << mLlr[evenIndex(mN, level - 1, 2 * node + 1)] << "]"
						  << std::endl;
#endif

				mOdd[oddIndex(mN, level - 1, leftGroup, 2 * node + 1)] =
						mOdd[oddIndex(mN, level, group, node)] +
						TemplatizedFloatCalc::F_function_calc(
							mEven[evenIndex(mN, level, node)],
							mLlr[evenIndex(mN, level - 1, 2 * node)]
							);
#ifdef SCAN_DEBUG_OUTPUT
				std::cout << "O(" << (level-1) << "," << leftGroup << "," << (2*node+1) << ") = "
						  << "O(" << level << "," << group << "," << node << ") + "
						  << "E(" << level << "," << node << ") BP "
						  << "L(" << (level-1) << "," << (2*node) << "): "
						  << mOdd[oddIndex(mN, level - 1, leftGroup, 2 * node + 1)] << " = "
						  << mOdd[oddIndex(mN, level, group, node)] << " + "
						  << mEven[evenIndex(mN, level, node)] << " BP "
						  << mLlr[evenIndex(mN, level - 1, 2 * node)]
						  << std::endl;
#endif
			}
		} else {
			for(unsigned node = 0; node < groupSize; ++node) {
				float t = mOdd[oddIndex(mN, level, group, node)]
						+ mLlr[evenIndex(mN, level - 1, 2 * node + 1)];
				mEven[evenIndex(mN, level - 1, 2 * node)] =
						TemplatizedFloatCalc::F_function_calc(
							mEven[evenIndex(mN, level, node)],
							t);
#ifdef SCAN_DEBUG_OUTPUT
				std::cout << "E(" << (level-1) << "," << (2*node) << ") = "
						  << "E(" << level << "," << node << ") BP ["
						  << "O(" << level << "," << group << "," << node << ") + "
						  << "L(" << (level-1) << "," << (2*node+1) << ")]: "
						  << mEven[evenIndex(mN, level - 1, 2 * node)] << " = "
						  << mEven[evenIndex(mN, level, node)] << " BP ["
						  << mOdd[oddIndex(mN, level, group, node)] << " + "
						  << mLlr[evenIndex(mN, level - 1, 2 * node + 1)] << "]"
						  << std::endl;
#endif

				mEven[evenIndex(mN, level - 1, 2 * node + 1)] =
						mOdd[oddIndex(mN, level, group, node)] +
						TemplatizedFloatCalc::F_function_calc(
							mEven[evenIndex(mN, level, node)],
							mLlr[evenIndex(mN, level - 1, 2 * node)]
							);
#ifdef SCAN_DEBUG_OUTPUT
				std::cout << "E(" << (level-1) << "," << (2*node+1) << ") = "
						  << "O(" << level << "," << group << "," << node << ") + "
						  << "E(" << level << "," << node << ") BP "
						  << "L(" << (level-1) << "," << (2*node) << "): "
						  << mEven[evenIndex(mN, level - 1, 2 * node + 1)] << " = "
						  << mOdd[oddIndex(mN, level, group, node)] << " + "
						  << mEven[evenIndex(mN, level, node)] << " BP "
						  << mLlr[evenIndex(mN, level - 1, 2 * node)]
						  << std::endl;
#endif
			}
		}
		if(leftGroup & 1) {
			updatebitmap(level - 1, leftGroup);
		}
	}
}

inline unsigned int bit_reverse(unsigned int v, unsigned int nbits) {
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	v = ( v >> 16             ) | ( v               << 16);
	v >>= (32-nbits);
	return v;
}

bool Scan::decode() {
	mLlr.assign(mLlr.size(), 0.0f);
	mEven.assign(mEven.size(), 0.0f);
	mOdd.assign(mOdd.size(), 0.0f);

	float *channelLlr = dynamic_cast<FloatContainer*>(mLlrContainer)->data();
	//L(0,0,i) LLRs from channel
	for(unsigned i = 0; i < mBlockLength; ++i) {
		mLlr[evenIndex(mN, 0, bit_reverse(i, mN))] = channelLlr[i];
	}

	for(unsigned i = 1; i < mBlockLength; i += 2) {
		if(mBooleanFrozen[i]) {
#ifdef SCAN_DEBUG_OUTPUT
			std::cout << "O(" << mN << "," << i << ",0) = inf" << std::endl;
#endif
			mOdd[oddIndex(mN, mN, i, 0)] = INFINITY;
		}
	}

	//Decoding
	for(unsigned i = 0; i < mIterationLimit; ++i) {
#ifdef SCAN_DEBUG_OUTPUT
		std::cout << "Iteration " << i << std::endl;
#endif
		for(unsigned group = 0; group < mBlockLength; ++group) {
#ifdef SCAN_DEBUG_OUTPUT
			std::cout << "Group " << group << std::endl;
#endif
			updatellrmap(mN, group);
			if(group & 1) {
				updatebitmap(mN, group);
			} else {
				if(mBooleanFrozen[group]) {
					mEven[evenIndex(mN, mN, 0)] = INFINITY;
#ifdef SCAN_DEBUG_OUTPUT
					std::cout << "E(" << mN << ",0) = inf" << std::endl;
#endif
				} else {
					mEven[evenIndex(mN, mN, 0)] = 0.0f;
#ifdef SCAN_DEBUG_OUTPUT
					std::cout << "E(" << mN << ",0) = 0" << std::endl;
#endif
				}
			}
		}
	}

	float *outputLlr = dynamic_cast<FloatContainer*>(mBitContainer)->data();

	//apply extrinsic LLRs
	for(unsigned i = 0; i < mBlockLength; i++) {
		unsigned idx = evenIndex(mN, 0, i);
		mLlr[idx] += mEven[idx];
	}

	for(unsigned i = 0; i < mBlockLength; i++) {
		outputLlr[i] = mLlr[evenIndex(mN, 0, bit_reverse(i, mN))];
	}

	mBitContainer->getPackedInformationBits(mOutputContainer);
	bool result = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size() + 7) / 8);
	return result;
}



}// namespace Decoding
}// namespace PolarCode
