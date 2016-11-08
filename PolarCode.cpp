#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

#include "PolarCode.h"
#include "ArrayFuncs.h"
#include "crc8.h"

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

inline float upperconv(float a, float b)
{
//	return logdomain_sum(a+b, 0.0) - logdomain_sum(a, b);
	return sgn(a) * sgn(b) * fmin(fabs(a), fabs(b));
}

inline float lowerconv(unsigned char bit, float upper, float lower)
{
	if(bit)
	{
		return lower-upper;
	}
	else
	{
		return lower+upper;
	}
}

PolarCode::PolarCode(int N, int K, int L, float designSNR)
{
	this->N = N;
	this->K = K;
	this->L = L;
	this->designSNR = designSNR;
	
	n = ceil(log2(N));
	N = 1<<n;//pow(2, n);
	
	//Initialize all the arrays
	FZLookup = new unsigned char[N]();
	bitreversed_indices = new int[N]();
	index_of_first0_from_MSB = new int[N]();
	index_of_first1_from_MSB = new int[N]();

	Metric = 0;
	PathCount = 0;
	LLR = 0;
	Bits = 0;
	Dhat = 0;
	NextLLR = 0;
	NextBits = 0;
	NextDhat = 0;
	NextMetric = 0;
	NextPaths = 0;
	memInitialized = false;

	resetMemory();	
	
	for(int i=0; i<N; ++i)
	{
		bitreversed_indices[i] = bitreversed_slow(i);
		int l = n-1;
		for(int k = 0; k<n; ++k)
		{
			if(i & (1<<(n-k-1)))
			{
				l = k;
				break;
			}
		}
		index_of_first1_from_MSB[i] = l;

		l = n-1;
		for(int k = 0; k<n; ++k)
		{
			if(!(i & (1<<(n-k-1))))
			{
				l = k;
				break;
			}
		}
		index_of_first0_from_MSB[i] = l;
	}
	
	pcc();
}
		
PolarCode::~PolarCode()
{
	delete [] FZLookup;
	delete [] bitreversed_indices;
	delete [] index_of_first0_from_MSB;
	delete [] index_of_first1_from_MSB;
}

void PolarCode::resetMemory()
{
	if(memInitialized)
	{
		delete[] Metric;
		PathCount = 0;
		for(int i=0; i<L; ++i)
		{
			delete[] LLR[i];
			delete[] Bits[i*2];
			delete[] Bits[i*2+1];
			delete[] Dhat[i];

			delete[] NextLLR[i];
			delete[] NextBits[(i<<1)*2];
			delete[] NextBits[(i<<1)*2+1];
			delete[] NextDhat[i<<1];
	
			delete[] NextBits[((i<<1)+1)*2];
			delete[] NextBits[((i<<1)+1)*2+1];
			delete[] NextDhat[(i<<1)+1];
		}
		delete[] LLR;
		delete[] Bits;
		delete[] Dhat;
		delete[] NextLLR;
		delete[] NextBits;
		delete[] NextDhat;
		delete[] NextMetric;
		delete[] NextPaths;
	}

	Metric = new float[L]();
	PathCount = 0;
	LLR = new float*[L];
	Bits = new unsigned char*[2*L];
	Dhat = new unsigned char*[L];
	NextLLR = new float*[L]();//null pointers
	NextBits = new unsigned char*[L*4]();
	NextDhat = new unsigned char*[L*2]();
	for(int i=0; i<L; ++i)
	{
		LLR[i] = new float[2*N-1]();
		Bits[i*2]   = new unsigned char[N-1]();
		Bits[i*2+1] = new unsigned char[N-1]();
		Dhat[i] = new unsigned char[N];

		NextLLR[i] = new float[2*N-1]();
		NextBits[(i<<1)*2]   = new unsigned char[N-1]();
		NextBits[(i<<1)*2+1] = new unsigned char[N-1]();
		NextDhat[i<<1] = new unsigned char[N];

		NextBits[((i<<1)+1)*2]   = new unsigned char[N-1]();
		NextBits[((i<<1)+1)*2+1] = new unsigned char[N-1]();
		NextDhat[(i<<1)+1] = new unsigned char[N];
	}
	NextMetric = new float[L*2]();
	NextPaths = new bool[L*2]();
	memInitialized = true;
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
	float *z = new float[N]();
	float designSNRlin = pow(10.0, designSNR/10.0);
	z[0] = -((double)K/N)*designSNRlin;
	
	
	float T; int B;
	for(int lev=0; lev < n; ++lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < B; ++j)
		{
			T = z[j];
			z[j] = logdomain_diff(log(2)+T, 2*T);
			z[j+B] = 2*T;
		}
	}
	
	trackingSorter sorter(z, N);
	sorter.sort();
	
	for(int i = 0; i<K; ++i)
	{
		FZLookup[sorter.permuted[i]] = 255;//Bit is available for user data
	}
	for(int i = K; i<N; ++i)
	{
		FZLookup[sorter.permuted[i]] = 0;//Set frozen bit to zero
	}
	
	delete[] z;
}

void PolarCode::encode(unsigned char *encoded, unsigned char *data)
{
	unsigned char *databits = new unsigned char[K]();
	unsigned char *codedbits = new unsigned char[N]();
	unsigned char checksum;
	CRC8 *CrcGenerator = new CRC8();
	
	//Calculate CRC
	checksum = CrcGenerator->generate(data, (K-8)>>3);
	
	//Split the data into bits
	Bytes2Bits(data, databits, (K>>3)-1);
	Bytes2Bits(&checksum, databits+K-8, 1);
	
	//Now insert the bits into Rate-1 channels
	//and frozen bits into Rate-0 channels
	unsigned char *bitptr = databits;
	for(int i=0; i<N; ++i)
	{
		if(FZLookup[i] == 255)
		{
			codedbits[i] = *bitptr;
			++bitptr;
		}
		else
		{
			codedbits[i] = FZLookup[i];
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
				codedbits[base+l] ^= codedbits[base+l +B];
			}
		}
	}
	
	//Merge bitstream into bytes
	Bits2Bytes(codedbits, encoded, N>>3);
	
	delete[] databits;
	delete[] codedbits;
	delete CrcGenerator;
}

bool PolarCode::decode(unsigned char *decodedBytes, float *initialLLR)
{
	resetMemory();
	trackingSorter *Sorter = new trackingSorter();
	CRC8 *Crc = new CRC8();
	//Initialize LLR-structure for first path
	float *LLRptr = LLR[0];
	for(int i=0; i<N-1; ++i)
	{
		*LLRptr = 0.0;
		++LLRptr;
	}
	for(int i=N-1; i<(N<<1)-1; ++i)
	{
		*LLRptr = *initialLLR;
		++LLRptr;
		++initialLLR;
	}
	PathCount = 1;
	
	
	for(int j=0; j<N; ++j)
	{
		int i = bitreversed_indices[j];
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			updateLLR(currentPath, i);
			
			if(FZLookup[i] == 255)
			{
				//Split current path into two new
				int fnextPath = currentPath<<1;//first
				int snextPath = fnextPath+1;//second
				
//				memcpy(NextLLR[fnextPath], LLR[currentPath], (2*N-1)*sizeof(float));
//				memcpy(NextLLR[snextPath], LLR[currentPath], (2*N-1)*sizeof(float));
				std::swap(NextLLR[currentPath], LLR[currentPath]);
				
				memcpy(NextBits[ fnextPath<<1   ], Bits[ currentPath<<1   ], N-1);
				memcpy(NextBits[(fnextPath<<1)+1], Bits[(currentPath<<1)+1], N-1);

//				memcpy(NextBits[ snextPath<<1   ], Bits[ currentPath<<1   ], N-1);
//				memcpy(NextBits[(snextPath<<1)+1], Bits[(currentPath<<1)+1], N-1);copy one, swap the other
				std::swap(NextBits[ snextPath<<1   ], Bits[ currentPath<<1   ]);
				std::swap(NextBits[(snextPath<<1)+1], Bits[(currentPath<<1)+1]);

				updateBits(fnextPath, 0, i);
				updateBits(snextPath, 1, i);
				
				memcpy(NextDhat[fnextPath], Dhat[currentPath], N);
//				memcpy(NextDhat[snextPath], Dhat[currentPath], N);copy one swap the other
				std::swap(NextDhat[snextPath], Dhat[currentPath]);
				NextDhat[fnextPath][i] = 0;
				NextDhat[snextPath][i] = 1;

				if(NextLLR[currentPath][0] > 0)
				{
					//Keep path metric, if LLR confirms the choice of d_hat
					NextMetric[fnextPath] = Metric[currentPath];
				}
				else
				{
					//Decrease this path's metric, if LLR tells that a 1 has been received more likely
					NextMetric[fnextPath] = Metric[currentPath] - fabs(NextLLR[currentPath][0]);
				}

				if(NextLLR[currentPath][0] < 0)
				{
					//Keep path metric, if LLR confirms the choice of d_hat
					NextMetric[snextPath] = Metric[currentPath];
				}
				else
				{
					//Decrease this path's metric, if LLR tells that a 1 has been received more likely
					NextMetric[snextPath] = Metric[currentPath] - fabs(NextLLR[currentPath][0]);
				}
				
				NextPaths[fnextPath] = true;
				NextPaths[snextPath] = true;
			}
			else
			{
				//Add only one path for frozen bit
				int nextPath = currentPath<<1;
				
//				memcpy(NextLLR[nextPath], LLR[currentPath], (2*N-1)*sizeof(float));
				std::swap(NextLLR[currentPath], LLR[currentPath]);
				
//				memcpy(NextBits[ nextPath<<1   ], Bits[ currentPath<<1   ], (N-1)*sizeof(unsigned char));
//				memcpy(NextBits[(nextPath<<1)+1], Bits[(currentPath<<1)+1], (N-1)*sizeof(unsigned char));

				std::swap(NextBits[ nextPath<<1   ], Bits[ currentPath<<1   ]);
				std::swap(NextBits[(nextPath<<1)+1], Bits[(currentPath<<1)+1]);

				updateBits(nextPath, 0, i);
				
//				memcpy(NextDhat[nextPath], Dhat[currentPath], N*sizeof(unsigned char));
				std::swap(NextDhat[nextPath], Dhat[currentPath]);
				NextDhat[nextPath][i] = 0;

				if(NextLLR[currentPath][0] > 0)
				{
					//Keep path metric, if LLR confirms the frozen bit
					NextMetric[nextPath] = Metric[currentPath];
				}
				else
				{
					NextMetric[nextPath] = Metric[currentPath] - fabs(NextLLR[currentPath][0]);
				}
				
				NextPaths[nextPath]   = true;
				NextPaths[nextPath+1] = false;
			}
		}
		
		int NumberOfNextPaths = 0;
		
		//Mark all the unused paths
		for(int i=0; i<2*L; ++i)
		{
			if(!NextPaths[i])
			{
				NextMetric[i] = -INFINITY;
			}
			else
			{
				NumberOfNextPaths++;
			}
		}
		
		Sorter->set(NextMetric, 2*L);
		Sorter->sort();
		
		PathCount = std::min(NumberOfNextPaths, L);
		for(int i=0; i<PathCount; ++i)
		{
			int path = Sorter->permuted[2*L-i-1];
			
			std::swap(LLR[i], NextLLR[path>>1]);
			std::swap(Bits[ i<<1   ], NextBits[ path<<1   ]);
			std::swap(Bits[(i<<1)+1], NextBits[(path<<1)+1]);
			std::swap(Dhat[i], NextDhat[path]);
			std::swap(Metric[i], NextMetric[path]);
		}
	}
	
	//Select the most likely path which passes the CRC test
	bool success = false;
	unsigned char *decodedBits = new unsigned char[K];
	unsigned char *decBytes = new unsigned char[K>>3];
	
	for(int path=0; path<PathCount; ++path)
	{
		unsigned char *bitptrA = decodedBits;
		unsigned char *bitptrB = Dhat[path];
		for(int bit=0; bit<N; ++bit)
		{
			if(FZLookup[bit] == 255)
			{
				*bitptrA = *bitptrB;
				++bitptrA;
			}
			++bitptrB;
		}
		Bits2Bytes(decodedBits, decBytes, K>>3);
		if(Crc->check(decBytes, (K>>3)-1, decBytes[(K>>3)-1]))
		{
			success = true;
			break;
		}
	}

	if(!success)//Select the most likely path, if all checks failed
	{
		unsigned char *bitptrA = decodedBits;
		unsigned char *bitptrB = Dhat[0];
		for(int bit=0; bit<N; ++bit)
		{
			if(FZLookup[bit] == 255)
			{
				*bitptrA = *bitptrB;
				++bitptrA;
			}
			++bitptrB;
		}
		Bits2Bytes(decodedBits, decBytes, K>>3);
	}
	
	memcpy(decodedBytes, decBytes, (K>>3)-1);
	
	delete[] decBytes;
	delete[] decodedBits;
	delete Crc;
	delete Sorter;
	
	return success;
}

void PolarCode::updateLLR(int path, int i)
{
	int nextlevel, lastlevel, st, ed, index;
	float *pathLLR = LLR[path];
	unsigned char *pathBits = Bits[path<<1];
	if(i==0)
	{
		nextlevel = n-1;
	}
	else
	{
		lastlevel = index_of_first1_from_MSB[i];
		
		st = 1<<lastlevel;
		ed = (1<<(lastlevel+1))-1;
		
		for(int idx = st; idx<=ed; ++idx)
		{
			index = ed+((idx-st)<<1);
			pathLLR[idx-1] = lowerconv(pathBits[idx-1],
			                           pathLLR[index],
									   pathLLR[index+1]);
		}
		nextlevel = lastlevel-1;
	}
	
	for(int lev=nextlevel; lev>=0; --lev)
	{
		st = 1<<lev;
		ed = (1<<(lev+1))-1;
		for(int idx=st; idx<=ed; ++idx)
		{
			index = ed+((idx-st)<<1);
			pathLLR[idx-1] = upperconv(pathLLR[index],
			                           pathLLR[index+1]);
		}
	}
}

void PolarCode::updateBits(int path, int d, int i)
{
	if(i==N-1)
	{
		return;
	}
	else if(i<(N>>1))
	{
		NextBits[path<<1][0] = d;
	}
	else
	{
		unsigned char *BitA = NextBits[path<<1],
		              *BitB = NextBits[(path<<1)+1];
		int lastlevel = index_of_first0_from_MSB[i];
		
		int lev, st, ed;
		BitB[0] = d;
		for(lev=0; lev<=lastlevel-2; ++lev)
		{
			st = 1<<lev;
			ed = (1<<(lev+1))-1;
			for(int idx=st; idx<=ed; ++idx)
			{
				BitB[ed+2*(idx-st)  ] = BitA[idx-1] ^ BitB[idx-1];
				BitB[ed+2*(idx-st)+1] = BitB[idx-1];
			}
		}
		
		lev = lastlevel-1;
		st = 1<<lev;
		ed = (1<<(lev+1))-1;
		for(int idx=st; idx<=ed; ++idx)
		{
			BitA[ed+2*(idx-st)  ] = BitA[idx-1] ^ BitB[idx-1];
			BitA[ed+2*(idx-st)+1] = BitB[idx-1];
		}
	}
}

