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

inline float lowerconv(bool bit, float upper, float lower)
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

PolarCode::PolarCode(int L, float designSNR)
{
	this->L = L;
	this->designSNR = designSNR;
	
	n = ceil(log2(PCparam_N));
	
	//Initialize all the arrays
	FZLookup.reserve(PCparam_N);
	bitreversed_indices.reserve(PCparam_N);
	index_of_first0_from_MSB.reserve(PCparam_N);
	index_of_first1_from_MSB.reserve(PCparam_N);

	resetMemory();	
	
	for(int i=0; i<PCparam_N; ++i)
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
	Metric.assign(L, 0.0);
	PathCount = 0;
	LLR.assign(L, vector<float>(2*PCparam_N-1, 0.0));
	Bits.assign(2*L, vector<bool>(PCparam_N-1, 0));
	Dhat.assign(L, vector<bool>(PCparam_N, 0));
	NextLLR.assign(L, vector<float>(2*PCparam_N-1, 0.0));
	NextBits.assign(4*L, vector<bool>(PCparam_N-1, 0));
	NextDhat.assign(2*L, vector<bool>(PCparam_N, 0));
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
	}
	for(int i = PCparam_K; i<PCparam_N; ++i)
	{
		FZLookup[sorter.permuted[i]] = false;//Freeze bit
	}
}

void PolarCode::encode(vector<bool> &encoded, vector<bool> &data)
{
	encoded.assign(PCparam_N, 0);
	CRC8 *CrcGenerator = new CRC8();
	
	//Calculate CRC
	CrcGenerator->addChecksum(data);
	
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
	
	delete CrcGenerator;
}

bool PolarCode::decode(vector<bool> &decoded, vector<float> &initialLLR)
{
	if(decodeOnePath(decoded, initialLLR))
	{
		return true;
	}
	else
	{
		return decodeMultiPath(decoded, initialLLR);
	}
}

bool PolarCode::decodeOnePath(vector<bool> &decoded, vector<float> &initialLLR)
{
	int backupL = L;
	L = 1;
	resetMemory();
	CRC8 *Crc = new CRC8();
	//Initialize LLR-structure for first path
	LLR[0].assign(PCparam_N-1, 0);//tree
	LLR[0].insert(LLR[0].end(), initialLLR.begin(), initialLLR.end());//data
	
	
	for(int j=0; j<PCparam_N; ++j)
	{
		int i = bitreversed_indices[j];

		updateLLR(0, i);
			
		if(FZLookup[i])
		{
			Dhat[0][i] = LLR[0][0] <= 0;

			std::swap(NextBits[0], Bits[0]);
			std::swap(NextBits[1], Bits[1]);

			updateBits(0, Dhat[0][i], i);

			std::swap(NextBits[0], Bits[0]);
			std::swap(NextBits[1], Bits[1]);		
		}
		else
		{
			Dhat[0][i] = 0;

			std::swap(NextBits[0], Bits[0]);
			std::swap(NextBits[1], Bits[1]);

			updateBits(0, 0, i);

			std::swap(NextBits[0], Bits[0]);
			std::swap(NextBits[1], Bits[1]);
		}
	}
	
	bool success = false;
	decoded.assign(PCparam_K, 0);
	
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

	success = Crc->check(decoded);
	
	L = backupL;

	delete Crc;
	
	return success;
}

bool PolarCode::decodeMultiPath(vector<bool> &decoded, vector<float> &initialLLR)
{
	resetMemory();
	trackingSorter *Sorter = new trackingSorter();
	CRC8 *Crc = new CRC8();
	//Initialize LLR-structure for first path
	LLR[0].assign(PCparam_N-1, 0);//tree
	LLR[0].insert(LLR[0].end(), initialLLR.begin(), initialLLR.end());//data
	PathCount = 1;
	
	
	float punishment;
	for(int j=0; j<PCparam_N; ++j)
	{
		int i = bitreversed_indices[j];
		NextMetric.assign(2*L, -INFINITY);
		int NumberOfNextPaths = 0;
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			updateLLR(currentPath, i);
			
			if(FZLookup[i])
			{
				//Split current path into two new
				int fnextPath = currentPath<<1;//first
				int snextPath = fnextPath+1;//second
				
				//std::swap(NextLLR[currentPath], LLR[currentPath]);
				NextLLR[currentPath] = LLR[currentPath];
				
				NextBits[ fnextPath<<1   ] = Bits[ currentPath<<1   ];
				NextBits[(fnextPath<<1)+1] = Bits[(currentPath<<1)+1];

				//std::swap(NextBits[ snextPath<<1   ], Bits[ currentPath<<1   ]);
				//std::swap(NextBits[(snextPath<<1)+1], Bits[(currentPath<<1)+1]);
				NextBits[ snextPath<<1   ] = Bits[ currentPath<<1   ];
				NextBits[(snextPath<<1)+1] = Bits[(currentPath<<1)+1];

				updateBits(fnextPath, 0, i);
				updateBits(snextPath, 1, i);
				
				NextDhat[fnextPath] = Dhat[currentPath];
				//std::swap(NextDhat[snextPath], Dhat[currentPath]);
				NextDhat[snextPath] = Dhat[currentPath];
				
				NextDhat[fnextPath][i] = 0;
				NextDhat[snextPath][i] = 1;
				
				punishment = fabs(NextLLR[currentPath][0]);

				if(NextLLR[currentPath][0] > 0)
				{
					//Decrease 1-path's metric, if LLR tells that a 1 has been received more likely
					NextMetric[fnextPath] = Metric[currentPath];
					NextMetric[snextPath] = Metric[currentPath] - punishment;
				}
				else
				{
					//Decrease 0-path's metric, if LLR tells that a 1 has been received more likely
					NextMetric[fnextPath] = Metric[currentPath] - punishment;
					NextMetric[snextPath] = Metric[currentPath];
				}
				
				NextPaths[fnextPath] = true;
				NextPaths[snextPath] = true;
				NumberOfNextPaths += 2;
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
				++NumberOfNextPaths;
			}
		}
		
		Sorter->set(NextMetric);
		Sorter->sort();
		
		PathCount = std::min(NumberOfNextPaths, L);
		for(int i=0; i<PathCount; ++i)
		{
			int path = Sorter->permuted[2*L-1-i];
			
			std::swap(LLR[i], NextLLR[path>>1]);
			std::swap(Bits[ i<<1   ], NextBits[ path<<1   ]);
			std::swap(Bits[(i<<1)+1], NextBits[(path<<1)+1]);
			std::swap(Dhat[i], NextDhat[path]);
			std::swap(Metric[i], NextMetric[path]);
		}
	}
	
	//Select the most likely path which passes the CRC test
	bool success = false;
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
	}

	if(!success)//Give out the most likely path, if no crc is fulfilled
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
	}

	delete Crc;
	delete Sorter;
	
	return success;
}

void PolarCode::updateLLR(int path, int i)
{
	int nextlevel, lastlevel, st, ed, idx, ctr;
	vector<float> &pathLLR = LLR[path];
	vector<bool> &pathBits = Bits[path<<1];
	if(i==0)
	{
		nextlevel = n-1;
	}
	else
	{
		lastlevel = index_of_first1_from_MSB[i];
		
		st = (1<<lastlevel)-1;
		ed = (1<<(lastlevel+1))-1;
		
		for(idx = st, ctr=ed; idx<ed; ++idx, ctr+=2)
		{
			pathLLR[idx] = lowerconv(pathBits[idx],
			                           pathLLR[ctr],
									   pathLLR[ctr+1]);
		}
		nextlevel = lastlevel-1;
	}
	
	for(int lev=nextlevel; lev>=0; --lev)
	{
		st = (1<<lev)-1;
		ed = (1<<(lev+1))-1;
		for(idx=st, ctr=ed; idx<ed; ++idx, ctr+=2)
		{
			pathLLR[idx] = upperconv(pathLLR[ctr],
			                         pathLLR[ctr+1]);
		}
	}
}

void PolarCode::updateBits(int path, int d, int i)
{
	if(i==PCparam_N-1)
	{
		return;
	}
	else if(i<(PCparam_N>>1))
	{
		NextBits[path<<1][0] = d;
	}
	else
	{
		vector<bool> &BitA = NextBits[path<<1],
		             &BitB = NextBits[(path<<1)+1];
		int lastlevel = index_of_first0_from_MSB[i];
		
		int lev, st, ed, idx, ctr;
		BitB[0] = d;
		for(lev=0; lev<=lastlevel-2; ++lev)
		{
			st = (1<<lev)-1;
			ed = (1<<(lev+1))-1;
			for(idx=st, ctr=ed; idx<ed; ++idx)
			{
				BitB[ctr++] = (BitA[idx] != BitB[idx]);
				BitB[ctr++] = BitB[idx];
			}
		}
		
		lev = lastlevel-1;
		st = (1<<lev)-1;
		ed = (1<<(lev+1))-1;
		for(idx=st, ctr=ed; idx<ed; ++idx)
		{
			BitA[ctr++] = (BitA[idx] != BitB[idx]);
			BitA[ctr++] = BitB[idx];
		}
	}
}

