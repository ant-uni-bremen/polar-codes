#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

#include <iostream>


#include "PolarCode.h"
#include "ArrayFuncs.h"
#include "crc8.h"

using namespace std;

inline float logdomain_sum(float x, float y)
{
	if(x<y)
	{
		return y+log1p(exp(x-y));
	}
	else
	{
		return x+log1p(exp(y-x));
	}
}

inline float logdomain_diff(float x, float y)
{
	return x+log1p(-exp(y-x));
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


void PolarCode::F_function(float *LLRin, float *LLRout, int size)
{
	size >>= 1;
	if(size <= 4)
	{
		float a,b;
		for(int i=0; i<size; ++i)
		{
			a = LLRin[i];
			b = LLRin[i+size];
			LLRout[i] = sgn(a) * sgn(b) * fmin(fabs(a),fabs(b));
		}
	}
	else
	{
		for(int i=0; i<size; i+=FLOATSPERVECTOR)
		{
			vec LLR_l = load_ps(LLRin+i);
			vec LLR_r = load_ps(LLRin+i+size);
			vec sign  = and_ps(xor_ps(LLR_l, LLR_r), SIGN_MASK);
			vec abs_l = andnot_ps(SIGN_MASK, LLR_l);
			vec abs_r = andnot_ps(SIGN_MASK, LLR_r);
			vec LLR_o = or_ps(sign, min_ps(abs_l, abs_r));
			store_ps(LLRout+i, LLR_o);
		}
	}
#ifdef DEBUGOUTPUT
	std::cout << "F function: ";
	for(int i=0; i<size; ++i)
	{
		std::cout << '(' << LLRin[i] << '#' << LLRin[i+size] << '=' << LLRout[i] << ')';
	}
	std::cout << endl;
#endif
}

void PolarCode::G_function(float *LLRin, float *LLRout, float *Bits, int size)
{
	size >>= 1;
	if(size <= 4)
	{
		unsigned int *FloatBit = reinterpret_cast<unsigned int*>(Bits);
		float tmp;unsigned int *iTmp = reinterpret_cast<unsigned int*>(&tmp);
		for(int i=0; i<size; ++i)
		{
			tmp = LLRin[i];
			*iTmp ^= FloatBit[i]&0x80000000;
			LLRout[i] = LLRin[i+size] + tmp;
		}
	}
	else
	{
		for(int i=0; i<size; i+=FLOATSPERVECTOR)
		{
			vec LLR_l  = load_ps(LLRin+i);
			vec LLR_r  = load_ps(LLRin+i+size);
			vec Bits_l = load_ps(Bits+i);
			vec LLR_n  = xor_ps(Bits_l, LLR_l);
			vec LLR_o = add_ps(LLR_r, LLR_n);
			store_ps(LLRout+i, LLR_o);
		}
	}
#ifdef DEBUGOUTPUT
	unsigned int *FloatBit = reinterpret_cast<unsigned int*>(Bits);
	std::cout << "G function: ";
	for(int i=0; i<size; ++i)
	{
		std::cout << '(' << LLRin[i+size] << (FloatBit[i]&0x80000000?'-':'+') << LLRin[i] << '=' << LLRout[i] << ')';
	}
	std::cout << endl;
#endif
}

void PolarCode::Rate0(float *BitsOut, int size)
{
	size>>=1;
	memset(BitsOut, 0, size*sizeof(float));
}

void PolarCode::Rate1(float *LLRin, float *BitsOut, int size)
{
	size>>=1;
	for(int i=0; i<size; ++i)
	{
		BitsOut[i] = (LLRin[i]<0)?-0.0:0.0;
	}
}

void PolarCode::Combine(float *BitsIn_l, float *BitsIn_r, float *BitsOut, int size)
{
#ifdef DEBUGOUTPUT
	std::cout << "Combine " << size << " elements: ";
#endif
	size >>= 1;
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec Bitsl = load_ps(BitsIn_l+i);
		vec Bitsr = load_ps(BitsIn_r+i);
		vec Bitso = xor_ps(Bitsl, Bitsr);
		store_ps(BitsOut+i, Bitso);
	}
	memcpy(BitsOut+size, BitsIn_r, sizeof(float)*size);
	
#ifdef DEBUGOUTPUT
	for(int i=0; i<size; ++i)
	{
		std::cout << '(' << BitsIn_l[i] << '^' << BitsIn_r[i] << '=' << BitsOut[i] << ')';
	}
	for(int i=0; i<size; ++i)
	{
		std::cout << '(' << BitsIn_r[i] << "=>" << BitsOut[i+size] << ')';
	}
	std::cout << endl;
#endif
}

void PolarCode::SPC(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iLLR = reinterpret_cast<unsigned int*>(LLRin);
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	bool parity = false;//false:Parity ok, true:Parity not ok
	int index = 0;
	float minLLR; unsigned int *iMinLLR = reinterpret_cast<unsigned int*>(&minLLR);
	float testLLR;unsigned int *iTestLLR = reinterpret_cast<unsigned int*>(&testLLR);
	*iMinLLR = iLLR[0]&0x7FFFFFFF;
	for(int i=0; i<size; ++i)
	{
		iBit[i]   = iLLR[i]&0x80000000;
		parity ^= !!iBit[i];

		*iTestLLR = iLLR[i]&0x7FFFFFFF;
		if(testLLR < minLLR)
		{
			index = i;
			minLLR = testLLR;
		}
	}

	//Flip least reliable bit, if neccessary
	if(parity)
	{
		//Flip least reliable bit
		iBit[index] ^= 0x80000000;
#ifdef DEBUGOUTPUT
		std::cout << "SPC flipped" << std::endl;
	}
	else
	{
		std::cout << "SPC ok" << std::endl;
	}
	std::cout << "SPC decided ";
	for(int i=0; i<size; ++i)
	{
		std::cout << '(' << LLRin[i] << "=>" << BitsOut[i] << ')';
	}
	std::cout << std::endl;
#else
	}
#endif
}

static inline float _mm256_reduce_add_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

void PolarCode::Repetition(float *LLRin, float *BitsOut, int size)
{
	if(size <= 4)
	{
		float sum = 0.0;
		for(int i=0; i<size; ++i)
		{
			sum += LLRin[i];
		}
		float bit = sum<0?-0.0:0.0;
		for(int i=0; i<size; ++i)
		{
			BitsOut[i] = bit;
		}
	}
	else
	{
		float HelperV=0;
		
		vec LLRsum = set1_ps(0.0);
		vec TmpVec;
		for(int i=0; i<size; i+=FLOATSPERVECTOR)
		{
			TmpVec = load_ps(LLRin+i);
			LLRsum = add_ps(LLRsum, TmpVec);
		}
		store_ps(AlignedVector, LLRsum);
		for(int i=0; i<FLOATSPERVECTOR; ++i)
		{
			HelperV += AlignedVector[i];
		}
		vec BitLLR = set1_ps(HelperV);
		vec BitDecision = and_ps(BitLLR, SIGN_MASK);
		for(int i=0; i<size; i+=FLOATSPERVECTOR)
		{
			store_ps(BitsOut+i, BitDecision);
		}
		
	}
#ifdef DEBUGOUTPUT
	std::cout << "Repetition decided ";
	for(int i=0; i<size; ++i)
	{
		std::cout << '(' << LLRin[i] << "=>" << BitsOut[i] << ')';
	}
	std::cout << std::endl;
#endif
}

PolarCode::PolarCode(int L, float designSNR)
{
	AlignedVector = (float*)_mm_malloc(FLOATSPERVECTOR * sizeof(float), sizeof(vec));
	SIGN_MASK = set1_ps(-0.0);
	this->L = L;
	this->designSNR = designSNR;
	
	n = ceil(log2(PCparam_N));

	//Initialize all the arrays
	FZLookup.reserve(PCparam_N);
	simplifiedTree.reserve(2*PCparam_N-1);

	resetMemory();
	
	
	pcc();
	
	
	

	
/*
	float *testLLR = (float*)_mm_malloc(16 * sizeof(float), sizeof(vec));
	float *testBits = (float*)_mm_malloc(16 * sizeof(float), sizeof(vec));

	testLLR[0] = 10;
	testLLR[1] = 5;
	testLLR[2] = 10;
	testLLR[3] = 10;
	testLLR[4] = 10;
	testLLR[5] = -3;
	testLLR[6] = 10;
	testLLR[7] = 10;
	testLLR[8] = 10;
	testLLR[9] = 10;
	testLLR[10] = 10;
	testLLR[11] = 10;
	testLLR[12] = -2;
	testLLR[13] = 10;
	testLLR[14] = 10;
	testLLR[15] = 10;

	
	Repetition(testLLR, testBits, 16);
	std::cout << "Repetition:" << std::endl;
	for(int i=0; i<16; ++i)
	{
		std::cout << testLLR[i] << " => " << testBits[i] << std::endl;
	}
	std::cout << std::endl;
	
	testLLR[0] = 10;
	testLLR[1] = -10;
	testLLR[2] = -10;
	testLLR[3] = 10;
	testLLR[4] = 10;
	testLLR[5] = -10;
	testLLR[6] = -10;
	testLLR[7] = 10;
	testLLR[8] = 10;
	testLLR[9] = -10;
	testLLR[10] = -10;
	testLLR[11] = 10;
	testLLR[12] = -2;
	testLLR[13] = -10;
	testLLR[14] = -10;
	testLLR[15] = 10;
	
	SPC(testLLR, testBits, 16);
	std::cout << "SPC:" << std::endl;
	for(int i=0; i<16; ++i)
	{
		std::cout << testLLR[i] << " => " << testBits[i] << std::endl;
	}
	std::cout << std::endl;
	
	F_function(testLLR, testBits, 16);
	std::cout << "F:" << std::endl;
	for(int i=0; i<16; ++i)
	{
		std::cout << testLLR[i] << " => " << testBits[i] << std::endl;
	}
	std::cout << std::endl;
	
	_mm_free(testLLR);
	_mm_free(testBits);
*/	
}
		
PolarCode::~PolarCode()
{
	_mm_free(AlignedVector);
}

void PolarCode::resetMemory()
{
	Metric.assign(L, 0.0);
	PathCount = 0;
	
	LLR.resize(L);
	Bits.resize(L);
	for(int list=0; list<L; ++list)
	{
		LLR[list].resize(n+1);
		Bits[list].resize(n+1);
		for(int stage=0; stage<=n; ++stage)
		{
			LLR[list][stage].assign(std::max(FLOATSPERVECTOR, 1<<stage), 0.0);		
			
			Bits[list][stage].resize(2);
			for(int leaf=0; leaf<2; ++leaf)
			{
				Bits[list][stage][leaf].assign(std::max(FLOATSPERVECTOR, 1<<stage), 0.0);
			}
		}
	}
	
	
	
	NextMetric.assign(L*2, 0.0);
	NextPaths.assign(L*2, 0);
}
	
unsigned int PolarCode::bitreversed_slow(unsigned int j)
{
	//Do a complete 32-bit-reversal
    j = (((j & 0xaaaaaaaa) >>  1) | ((j & 0x55555555) <<  1));
    j = (((j & 0xcccccccc) >>  2) | ((j & 0x33333333) <<  2));
    j = (((j & 0xf0f0f0f0) >>  4) | ((j & 0x0f0f0f0f) <<  4));
    j = (((j & 0xff00ff00) >>  8) | ((j & 0x00ff00ff) <<  8));
    j = (( j               >> 16) | ( j               << 16));
    return j>>(32-n);//return the reversed n-bit value
}

void PolarCode::pcc()
{
	vector<float> z(PCparam_N, 0.0);
	float designSNRlin = pow(10.0, designSNR/10.0);
	z[0] = -((double)PCparam_K/PCparam_N)*designSNRlin;
	
	
	float T; int B;
	for(int lev=0; lev < n; ++lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < B; ++j)
		{
			T = z[j];
			z[j] = logdomain_diff(log(2.0)+T, 2*T);
			z[j+B] = 2*T;
		}
	}
	
	trackingSorter sorter(z);
	sorter.sort();
	
	for(int i = 0; i<PCparam_K; ++i)
	{
		FZLookup[sorter.permuted[i]] = true;//Bit is available for user data
		simplifiedTree[PCparam_N-1+sorter.permuted[i]] = nodeInfo::RateOne;
	}
	for(int i = PCparam_K; i<PCparam_N; ++i)
	{
		FZLookup[sorter.permuted[i]] = false;//Freeze bit
		simplifiedTree[PCparam_N-1+sorter.permuted[i]] = nodeInfo::RateZero;
	}
	
	for(int lev=n-1; lev>=0; --lev)
	{
		int st = (1<<lev)-1;
		int ed = (1<<(lev+1))-1;
		int idx, ctr;
		for(idx=st, ctr=ed; idx<ed; ++idx)
		{
			nodeInfo Left  = simplifiedTree[ctr++];
			nodeInfo Right = simplifiedTree[ctr++];
			if(Left == RateZero && Right == RateZero)
			{
				simplifiedTree[idx] = RateZero;
			}
			else if(Left == RateOne && Right == RateOne)
			{
				simplifiedTree[idx] = RateOne;
			}
			else if((Left == RateHalf || Left == SPCnode) && Right == RateOne)
			{
				simplifiedTree[idx] = SPCnode;
			}
			else if(Left == RateZero && (Right == RateHalf || Right == RepetitionNode))
			{
				simplifiedTree[idx] = RepetitionNode;
			}
			else if(Left == RateZero && Right == RateOne && lev==n-1)
			{
				simplifiedTree[idx] = RateHalf;
			}
			else
			{
				simplifiedTree[idx] = RateR;
			}
/*			std::cout << "Left " << (ctr-2) << " is ";
			switch(Left)
			{
			case RateZero:
				std::cout << "Rate-0";break;
			case RateOne:
				std::cout << "Rate-1";break;
			case RateHalf:
				std::cout << "Rate-0.5";break;
			case RepetitionNode:
				std::cout << "Repetition";break;
			case SPCnode:
				std::cout << "SPC";break;
			case RateR:
				std::cout << "Rate-R";break;
			default:
				std::cout << "Error";break;
			}
			std::cout << std::endl;
			std::cout << "Right " << (ctr-1) << " is ";
			switch(Right)
			{
			case RateZero:
				std::cout << "Rate-0";break;
			case RateOne:
				std::cout << "Rate-1";break;
			case RateHalf:
				std::cout << "Rate-0.5";break;
			case RepetitionNode:
				std::cout << "Repetition";break;
			case SPCnode:
				std::cout << "SPC";break;
			case RateR:
				std::cout << "Rate-R";break;
			default:
				std::cout << "Error";break;
			}
			std::cout << std::endl;
			std::cout << "Parent " << (idx) << " is ";
			switch(simplifiedTree[idx])
			{
			case RateZero:
				std::cout << "Rate-0";break;
			case RateOne:
				std::cout << "Rate-1";break;
			case RateHalf:
				std::cout << "Rate-0.5";break;
			case RepetitionNode:
				std::cout << "Repetition";break;
			case SPCnode:
				std::cout << "SPC";break;
			case RateR:
				std::cout << "Rate-R";break;
			default:
				std::cout << "Error";break;
			}
			std::cout << std::endl;

*/


		}

	}
	
/*	bool nonRfound = false;
	for(int i=0; i<PCparam_N-1; ++i)
	{
		std::cout << "Stage " << floor(log2(i+1)-1) << ", i=" << i << ": ";
		switch(simplifiedTree[i])
		{
		case RateZero:
			std::cout << "Rate-0";break;
		case RateOne:
			std::cout << "Rate-1";break;
		case RateHalf:
			std::cout << "Rate-0.5";break;
		case RepetitionNode:
			std::cout << "Repetition";break;
		case SPCnode:
			std::cout << "SPC";break;
		case RateR:
			std::cout << "Rate-R";break;
		default:
			std::cout << "Error";break;
		}
		std::cout << std::endl;
		if(simplifiedTree[i] != RateR && !nonRfound)
		{
			nonRfound = true;
			std::cout << "First non-rate-R node!" << std::endl;
		}
	}*/
}

void PolarCode::encode(vector<bool> &encoded, vector<bool> &data)
{
	encoded.assign(PCparam_N, 0);
#ifdef CRCSIZE
	CRC8 *CrcGenerator = new CRC8();
	
	//Calculate CRC
	CrcGenerator->addChecksum(data);
#endif
	
	//Insert the bits into Rate-1 channels
	//and frozen bits into Rate-0 channels
	auto bitptr = data.begin();
	for(int i=0; i<PCparam_N; ++i)
	{
		if(FZLookup[i])
		{
			encoded[i] = *bitptr;
			++bitptr;
		}
	}
	
	//Encode
	int B, nB, base;
	for(int i=0; i<n; ++i)
	{
		B = 1<<(n-i-1);
		nB = 1<<i;
		for(int j=0; j<nB; ++j)
		{
			base = j*(B<<1);
			for(int l=0; l<B; ++l)
			{
				encoded[base+l] = (encoded[base+l] != encoded[base+l +B]);// here '!=' works as XOR-operator. '^' doesn't...
			}
		}
	}
#ifdef CRCSIZE	
	delete CrcGenerator;
#endif
}

void PolarCode::transform(aligned_float_vector &BitsIn, vector<bool> &BitsOut)
{
	int size = BitsOut.size();
	unsigned int *FloatBit = reinterpret_cast<unsigned int *>(BitsIn.data());
	for(int i=0; i<size; ++i)
	{
		BitsOut[i] = FloatBit[i]&0x80000000;
	}

#ifdef DEBUGOUTPUT
	std::cout << "Tf ";
	for(int i=0; i<size; ++i)
	{
		std::cout << BitsOut[i];
	}
	std::cout << std::endl;
#endif

	int B, nB, base;
	for(int i=0; i<n; ++i)
	{
		B = 1<<(n-i-1);
		nB = 1<<i;
		for(int j=0; j<nB; ++j)
		{
			base = j*(B<<1);
			for(int l=0; l<B; ++l)
			{
				BitsOut[base+l] = (BitsOut[base+l] != BitsOut[base+l +B]);// here '!=' works as XOR-operator. '^' doesn't...
			}
		}
	}
	
#ifdef DEBUGOUTPUT
	std::cout << "to ";
	for(int i=0; i<size; ++i)
	{
		std::cout << BitsOut[i];
	}
	std::cout << std::endl;
#endif
}

bool PolarCode::decode(vector<bool> &decoded, vector<float> &initialLLR)
{
	if(decodeOnePath(decoded, initialLLR))
	{
		return true;
	}
	else
	{
		return false; //decodeMultiPath(decoded, initialLLR);
	}
}

bool PolarCode::decodeOnePath(vector<bool> &decoded, vector<float> &initialLLR)
{
	int backupL = L;
	L = 1;
	resetMemory();
#ifdef CRCSIZE
	CRC8 *Crc = new CRC8();
#endif
	for(int i=0; i<PCparam_N; ++i)
	{
		/* copy element by element to the aligned storage */
		LLR[0][n][i] = initialLLR[i];
	}
	
	decodeOnePathRecursive(n,0,0);
	
	vector<bool> Dhat(PCparam_N);
	
	transform(Bits[0][n][0], Dhat);

	auto bitptrA = decoded.begin();
	auto bitptrB = Dhat.begin();
	for(int bit=0; bit<PCparam_N; ++bit)
	{
		if(FZLookup[bit])
		{
			*bitptrA = *bitptrB;
			++bitptrA;
		}
		++bitptrB;
	}
#ifdef CRCSIZE
	bool success = Crc->check(decoded);	
	delete Crc;
#else
	bool success = true;
#endif


	L = backupL;

	
	return success;
}

void PolarCode::decodeOnePathRecursive(int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int stageLength = 1<<stage;
	int subStageLength = 1<<(stage-1);
	if(stage != 0)
	{	
		F_function(LLR[0][stage].data(), LLR[0][stage-1].data(), stageLength);
		switch(simplifiedTree[leftNode])
		{
		case RateZero:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Left node " << leftNode << " is rate zero of length " << subStageLength << "." << std::endl;
#endif
			Rate0(Bits[0][stage-1][0].data(), subStageLength);
			break;
		case RateOne:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Left node " << leftNode << " is rate one of length " << subStageLength << "." << std::endl;
#endif
			Rate1(LLR[0][stage-1].data(), Bits[0][stage-1][0].data(), subStageLength);
			break;
		case RepetitionNode:
		case RateHalf:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Left node " << leftNode << " is repetition of length " << subStageLength << "." << std::endl;
#endif
			Repetition(LLR[0][stage-1].data(), Bits[0][stage-1][0].data(), subStageLength);
			break;
		case SPCnode:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Left node " << leftNode << " is single parity of length " << subStageLength << "." << std::endl;
#endif
			SPC(LLR[0][stage-1].data(), Bits[0][stage-1][0].data(), subStageLength);
			break;
		default:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Left node " << leftNode << " is rate R of length " << subStageLength << "." << std::endl;
#endif
			decodeOnePathRecursive(stage-1, 0, leftNode);
		}

		G_function(LLR[0][stage].data(), LLR[0][stage-1].data(), Bits[0][stage-1][0].data(), stageLength);
		switch(simplifiedTree[rightNode])
		{
		case RateZero:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Right node " << rightNode << " is rate zero of length " << subStageLength << "." << std::endl;
#endif
			Rate0(Bits[0][stage-1][1].data(), subStageLength);
			break;
		case RateOne:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Right node " << rightNode << " is rate one of length " << subStageLength << "." << std::endl;
#endif
			Rate1(LLR[0][stage-1].data(), Bits[0][stage-1][1].data(), subStageLength);
			break;
		case RepetitionNode:
		case RateHalf:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Right node " << rightNode << " is repetition of length " << subStageLength << "." << std::endl;
#endif
			Repetition(LLR[0][stage-1].data(), Bits[0][stage-1][1].data(), subStageLength);
			break;
		case SPCnode:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Right node " << rightNode << " is single parity of length " << subStageLength << "." << std::endl;
#endif
			SPC(LLR[0][stage-1].data(), Bits[0][stage-1][1].data(), subStageLength);
			break;
		default:
#ifdef DEBUGOUTPUT
			std::cout << nodeID << ": Right node " << rightNode << " is rate R of length " << subStageLength << "." << std::endl;
#endif
			decodeOnePathRecursive(stage-1, 1, rightNode);
		}
		Combine(Bits[0][stage-1][0].data(), Bits[0][stage-1][1].data(), Bits[0][stage][BitLocation].data(), 1<<stage);
	}
	else
	{
#ifdef DEBUGOUTPUT
		std::cout << nodeID << ": This is a last stage bit." << std::endl;
#endif
		//Rather complex looking bit decision by reducing the float to its sign bit
		float number = LLR[0][0][0];
		unsigned int *A = reinterpret_cast<unsigned int *>(&number);
		Bits[0][0][BitLocation][0] = *A & 0x80000000;
#ifdef DEBUGOUTPUT
		std::cout << nodeID << ": Decided " << Bits[0][0][BitLocation][0] << std::endl;
#endif
	}
}

bool PolarCode::decodeMultiPath(vector<bool> &decoded, vector<float> &initialLLR)
{
	resetMemory();
	trackingSorter *Sorter = new trackingSorter();
	CRC8 *Crc = new CRC8();
	//Initialize LLR-structure for first path
	for(int i=0; i<PCparam_N; ++i)
	{
		/* copy element by element to the aligned storage */
		LLR[0][n][i] = initialLLR[i];
	}
	PathCount = 1;
	
	

	
	//Select the most likely path which passes the CRC test
/*	bool success = false;
	decoded.assign(PCparam_K, 0);
	
	for(int path=0; path<PathCount; ++path)
	{
		auto bitptrA = decoded.begin();
		auto bitptrB = Dhat[path].begin();
		for(int bit=0; bit<PCparam_N; ++bit)
		{
			if(FZLookup[bit])
			{
				*bitptrA = *bitptrB;
				++bitptrA;
			}
			++bitptrB;
		}
		if(Crc->check(decoded))
		{
			success = true;
			break;
		}
	}*/

/*	if(!success)//Give out the most likely path, if no crc is fulfilled
	{
		auto bitptrA = decoded.begin();
		auto bitptrB = Dhat[0].begin();
		for(int bit=0; bit<PCparam_N; ++bit)
		{
			if(FZLookup[bit])
			{
				*bitptrA = *bitptrB;
				++bitptrA;
			}
			++bitptrB;
		}
	}*/

	delete Crc;
	delete Sorter;
	
	return false;//success;
}


