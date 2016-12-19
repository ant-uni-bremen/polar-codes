#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>

#define SPECIALPARAMETERS_H
#define PCparam_N 128
#define PCparam_K 72

#include "../ArrayFuncs.h"

const float designSNR = 5.0;

enum nodeInfo
{
	RateZero,
	RateOne,
	RateHalf,
	RepetitionNode,
	SPCnode,
	RepSPCnode,
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
	

	if(simplifiedTree[leftNode] != RateZero)
	{
		File << "F_function" << vectorized << "(LLR[0][" << stage << "].data(), LLR[0][" << (stage-1) << "].data(), " << subStageLength << ");" << endl;
	}

	switch(simplifiedTree[leftNode])
	{
	case RateZero:
//		File << "Rate0(Bits[0][" << (stage-1) << "][0].data(), " << subStageLength << ");" << endl;
		break;
	case RateOne:
		File << "Rate1(LLR[0][" << (stage-1) << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		break;
	case RepetitionNode:
	case RateHalf:
		File << "Repetition" << vectorized << "(LLR[0][" << (stage-1) << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		break;
	case SPCnode:
		File << "SPC(LLR[0][" << (stage-1) << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		break;
	case RepSPCnode:
		if(subStageLength == 8)
		{
			File << "RepSPC_8(LLR[0][" << (stage-1) << "].data(), BitPtr+" << BitLocation << ");" << endl;
		}
		else
		{
			File << "RepSPC(LLR[0][" << (stage-1) << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		}
		break;
	default:
		printDecoder(stage-1, BitLocation, leftNode);
	}


	if(simplifiedTree[rightNode] == RateOne)
	{
		if(simplifiedTree[leftNode] == RateZero)
		{
			File << "P_01(LLR[0][" << stage << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		}
		else
		{
			File << "P_R1(LLR[0][" << stage << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		}
	}
	else
	{
		if(simplifiedTree[leftNode] != RateZero)
		{
			File << "G_function" << vectorized << "(LLR[0][" << stage << "].data(), LLR[0][" << (stage-1) << "].data(), BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		}
		else
		{
			File << "G_function_0R" << vectorized << "(LLR[0][" << stage << "].data(), LLR[0][" << (stage-1) << "].data(), " << subStageLength << ");" << endl;
		}
		
		switch(simplifiedTree[rightNode])
		{
		case RateZero:
			File << "Rate0(BitPtr+" << (BitLocation+subStageLength) << ", " << subStageLength << ");" << endl;
			cout << "Right rate 0, left is " << simplifiedTree[leftNode] << "!";
			break;
/*		case RateOne:
			File << "Rate1(LLR[0][" << (stage-1) << "].data(), BitPtr+" << (BitLocation+subStageLength) << ", " << subStageLength << ");" << endl;
			break;*/
		case RepetitionNode:
		case RateHalf:
			File << "Repetition" << vectorized << "(LLR[0][" << (stage-1) << "].data(), BitPtr+" << (BitLocation+subStageLength) << ", " << subStageLength << ");" << endl;
			cout << "Right repetition";
			break;
		case SPCnode:
			File << "SPC(LLR[0][" << (stage-1) << "].data(), BitPtr+" << (BitLocation+subStageLength) << ", " << subStageLength << ");" << endl;
			break;
		case RepSPCnode:
			if(subStageLength == 8)
			{
				File << "RepSPC_8(LLR[0][" << (stage-1) << "].data(), BitPtr+" << (BitLocation+subStageLength) << ");" << endl;
			}
			else
			{
				File << "RepSPC(LLR[0][" << (stage-1) << "].data(), BitPtr+" << (BitLocation+subStageLength) << ", " << subStageLength << ");" << endl;
			}
			break;
		default:
			printDecoder(stage-1, BitLocation+subStageLength, rightNode);
		}
		
		if(simplifiedTree[leftNode] != RateZero)
		{
			File << "CombineSimple(BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		}
		else
		{
			File << "Combine_0RSimple(BitPtr+" << BitLocation << ", " << subStageLength << ");" << endl;
		}
	}
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
	int n = log2(PCparam_N);
	FZLookup.reserve(PCparam_N);
	simplifiedTree.reserve(2*PCparam_N-1);

	vector<float> z(PCparam_N, 0.0);
	float designSNRlin = pow(10.0, designSNR/10.0);
	z[0] = -((double)PCparam_K/PCparam_N)*designSNRlin;
	
	cout << "Initial z[0] = " << z[0] << endl;
	
	
	float T; int B;
/*	for(int lev=0; lev < n; ++lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < B; ++j)
		{
			T = zz[j];
			zz[j] = logdomain_diff(log(2.0)+T, 2*T);
			zz[j+B] = 2*T;
		}
	}*/

	for(int lev=n-1; lev >= 0; --lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < PCparam_N; j+=(B<<1))
		{
			T = z[j];
			z[j] = logdomain_diff(log(2.0)+T, 2*T);
			z[j+B] = 2*T;
		}
	}

	
/*	for(int i=0; i<PCparam_N; ++i)
	{
		z[i] = zz[bitreversed_slow(i)];
	}*/
	
	trackingSorter sorter(z);
	sorter.stableSort();
	
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
			else if(Left == RepetitionNode && Right == SPCnode)
			{
				simplifiedTree[idx] = RepSPCnode;
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
