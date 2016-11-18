#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>

#include "../ArrayFuncs.h"

const float designSNR = 0.0;
#define PCparam_N 1024
#define PCparam_K 512

enum nodeInfo
{
	RateZero,
	RateOne,
	RateHalf,
	RepetitionNode,
	SPCnode,
	RateR
};

using namespace std;

ofstream File;
vector<bool> FZLookup;
vector<nodeInfo> simplifiedTree;


inline float logdomain_diff(float x, float y)
{
	return x+log1p(-exp(y-x));
}

void printDecoder(int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);
	
	string vectorized = "";
	if(subStageLength >= 8)
	{
		vectorized = "_vectorized";
	}
	

	File << "F_function" << vectorized << "(LLR[0][" << stage << "].data(), LLR[0][" << (stage-1) << "].data(), " << subStageLength << ");" << endl;

	switch(simplifiedTree[leftNode])
	{
	case RateZero:
		File << "Rate0(Bits[0][" << (stage-1) << "][0].data(), " << subStageLength << ");" << endl;
		break;
	case RateOne:
		File << "Rate1(LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][0].data(), " << subStageLength << ");" << endl;
		break;
	case RepetitionNode:
	case RateHalf:
		File << "Repetition" << vectorized << "(LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][0].data(), " << subStageLength << ");" << endl;
		break;
	case SPCnode:
		File << "SPC(LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][0].data(), " << subStageLength << ");" << endl;
		break;
	default:
		printDecoder(stage-1, 0, leftNode);
	}

	File << "G_function" << vectorized << "(LLR[0][" << stage << "].data(), LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][0].data(), " << subStageLength << ");" << endl;
	switch(simplifiedTree[rightNode])
	{
	case RateZero:
		File << "Rate0(Bits[0][" << (stage-1) << "][1].data(), " << subStageLength << ");" << endl;
		break;
	case RateOne:
		File << "Rate1(LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][1].data(), " << subStageLength << ");" << endl;
		break;
	case RepetitionNode:
	case RateHalf:
		File << "Repetition" << vectorized << "(LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][1].data(), " << subStageLength << ");" << endl;
		break;
	case SPCnode:
		File << "SPC(LLR[0][" << (stage-1) << "].data(), Bits[0][" << (stage-1) << "][1].data(), " << subStageLength << ");" << endl;
		break;
	default:
		printDecoder(stage-1, 1, rightNode);
	}
	
	File << "Combine(Bits[0][" << (stage-1) << "][0].data(), Bits[0][" << (stage-1) << "][1].data(), Bits[0][" << stage << "][" << BitLocation << "].data(), " << subStageLength << ");" << endl;
} 


unsigned int bitreversed_slow(unsigned int j, unsigned int n)
{
	//Do a complete 32-bit-reversal
    j = (((j & 0xaaaaaaaa) >>  1) | ((j & 0x55555555) <<  1));
    j = (((j & 0xcccccccc) >>  2) | ((j & 0x33333333) <<  2));
    j = (((j & 0xf0f0f0f0) >>  4) | ((j & 0x0f0f0f0f) <<  4));
    j = (((j & 0xff00ff00) >>  8) | ((j & 0x00ff00ff) <<  8));
    j = (( j               >> 16) | ( j               << 16));
    return j>>(32-n);//return the reversed n-bit value
}

int main(void)
{
	unsigned int n = log2(PCparam_N);
	FZLookup.reserve(PCparam_N);
	simplifiedTree.reserve(2*PCparam_N-1);

	vector<float> zz(PCparam_N, 0.0);
	vector<float> z(PCparam_N, 0.0);
	float designSNRlin = pow(10.0, designSNR/10.0);
	zz[0] = -((double)PCparam_K/PCparam_N)*designSNRlin;
	
	
	float T; int B;
	for(int lev=0; lev < n; ++lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < B; ++j)
		{
			T = zz[j];
			zz[j] = logdomain_diff(log(2.0)+T, 2*T);
			zz[j+B] = 2*T;
		}
	}
	
	for(int i=0; i<PCparam_N; ++i)
	{
		z[i] = zz[bitreversed_slow(i,n)];
	}
	
	trackingSorter sorter(z);
	sorter.sort();
	
	for(int i = 0; i<PCparam_K; ++i)
	{
		simplifiedTree[PCparam_N-1+sorter.permuted[i]] = nodeInfo::RateOne;
	}
	for(int i = PCparam_K; i<PCparam_N; ++i)
	{
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
		}
	}
	
	File.open("../FZLookup.cpp");
	
	File << "bool bFZLookup[" << PCparam_N << "] = {";
	for(int i=0; i<PCparam_N; ++i)
	{
		File << FZLookup[i];
		if(i+1 != PCparam_N)
		{
			File << ',';
		}
	}
	File << "};" << endl;
	File.close();
	
	File.open("../SpecialDecoder.cpp");
	printDecoder(n, 0, 0);
	File.close();
	
	File.open("../SpecialParameters.h");
	File
	<< "#ifndef SPECIALPARAMETERS_H" << endl
	<< "#define SPECIALPARAMETERS_H" << endl
	<< "#define PCparam_N " << PCparam_N << endl
	<< "#define PCparam_K " << PCparam_K << endl
	<< "#endif" << endl;
	File.close();
	return 0;
}
