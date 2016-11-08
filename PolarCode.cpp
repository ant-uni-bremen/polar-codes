#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

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
	FZLookup.reserve(N);
	bitreversed_indices.reserve(N);
	index_of_first0_from_MSB.reserve(N);
	index_of_first1_from_MSB.reserve(N);

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
}

void PolarCode::resetMemory()
{
	Metric.assign(L, 0);
	PathCount = 0;
	LLR.assign(L, vector<float>(2*N-1,0.0));
	Bits.assign(2*L, vector<bool>(N-1, 0));
	Dhat.assign(L, vector<bool>(N, 0));
	NextLLR.assign(L, vector<float>(2*N-1, 0));
	NextBits.assign(4*L, vector<bool>(N-1, 0));
	NextDhat.assign(2*L, vector<bool>(N, 0));
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
	vector<float> z(N, 0.0);
	float designSNRlin = pow(10.0, designSNR/10.0);
	z[0] = -((double)K/N)*designSNRlin;
	
	
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
	
	for(int i = 0; i<K; ++i)
	{
		FZLookup[sorter.permuted[i]] = 255;//Bit is available for user data
	}
	for(int i = K; i<N; ++i)
	{
		FZLookup[sorter.permuted[i]] = 0;//Set frozen bit to zero
	}
}

void PolarCode::encode(vector<bool> &encoded, vector<bool> &data)
{
	encoded.assign(N, 0);
	CRC8 *CrcGenerator = new CRC8();
	
	//Calculate CRC
	CrcGenerator->addChecksum(data);
	
	//Insert the bits into Rate-1 channels
	//and frozen bits into Rate-0 channels
	auto bitptr = data.begin();
	for(int i=0; i<N; ++i)
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
	
	delete CrcGenerator;
}

bool PolarCode::decode(vector<bool> &decoded, vector<float> &initialLLR)
{
	resetMemory();
	trackingSorter *Sorter = new trackingSorter();
	CRC8 *Crc = new CRC8();
	//Initialize LLR-structure for first path
	LLR[0].assign(N-1, 0);//tree
	LLR[0].insert(LLR[0].end(), initialLLR.begin(), initialLLR.end());//data
	PathCount = 1;
	
	
	for(int j=0; j<N; ++j)
	{
		int i = bitreversed_indices[j];
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			updateLLR(currentPath, i);
			
			if(FZLookup[i])
			{
				//Split current path into two new
				int fnextPath = currentPath<<1;//first
				int snextPath = fnextPath+1;//second
				
				std::swap(NextLLR[currentPath], LLR[currentPath]);
				
				NextBits[ fnextPath<<1   ] = Bits[ currentPath<<1   ];
				NextBits[(fnextPath<<1)+1] = Bits[(currentPath<<1)+1];

				std::swap(NextBits[ snextPath<<1   ], Bits[ currentPath<<1   ]);
				std::swap(NextBits[(snextPath<<1)+1], Bits[(currentPath<<1)+1]);

				updateBits(fnextPath, 0, i);
				updateBits(snextPath, 1, i);
				
				NextDhat[fnextPath] = Dhat[currentPath];
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
				
				std::swap(NextLLR[currentPath], LLR[currentPath]);
				
				std::swap(NextBits[ nextPath<<1   ], Bits[ currentPath<<1   ]);
				std::swap(NextBits[(nextPath<<1)+1], Bits[(currentPath<<1)+1]);

				updateBits(nextPath, 0, i);
				
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
		
		Sorter->set(NextMetric);
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
	decoded.assign(K, 0);
	
	for(int path=0; path<PathCount; ++path)
	{
		auto bitptrA = decoded.begin();
		auto bitptrB = Dhat[path].begin();
		for(int bit=0; bit<N; ++bit)
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
	}

	if(!success)//Select the most likely path, if all checks failed
	{
		auto bitptrA = decoded.begin();
		auto bitptrB = Dhat[0].begin();
		for(int bit=0; bit<N; ++bit)
		{
			if(FZLookup[bit] == 255)
			{
				*bitptrA = *bitptrB;
				++bitptrA;
			}
			++bitptrB;
		}
	}

	delete Crc;
	delete Sorter;
	
	return success;
}

void PolarCode::updateLLR(int path, int i)
{
	int nextlevel, lastlevel, st, ed, index;
	vector<float> &pathLLR = LLR[path];
	vector<bool> &pathBits = Bits[path<<1];
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
		vector<bool> &BitA = NextBits[path<<1],
		             &BitB = NextBits[(path<<1)+1];
		int lastlevel = index_of_first0_from_MSB[i];
		
		int lev, st, ed;
		BitB[0] = d;
		for(lev=0; lev<=lastlevel-2; ++lev)
		{
			st = 1<<lev;
			ed = (1<<(lev+1))-1;
			for(int idx=st; idx<=ed; ++idx)
			{
				BitB[ed+2*(idx-st)  ] = (BitA[idx-1] != BitB[idx-1]);
				BitB[ed+2*(idx-st)+1] = BitB[idx-1];
			}
		}
		
		lev = lastlevel-1;
		st = 1<<lev;
		ed = (1<<(lev+1))-1;
		for(int idx=st; idx<=ed; ++idx)
		{
			BitA[ed+2*(idx-st)  ] = (BitA[idx-1] != BitB[idx-1]);
			BitA[ed+2*(idx-st)+1] = BitB[idx-1];
		}
	}
}

