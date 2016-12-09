#include "ArrayFuncs.h"

#include <cassert>
#include <algorithm>
#include <utility>

trackingSorter::trackingSorter()
{
	unset();
}

trackingSorter::trackingSorter(std::vector<float> &arr)
{
	set(arr);
}

trackingSorter::~trackingSorter()
{
	unset();
}

void trackingSorter::set(std::vector<float> &arr)
{
	unset();
	size = arr.size();
	sorted = arr;
	permuted.reserve(size);
	for(int i=0; i<size; ++i)
	{
		permuted[i] = i;
	}
}

void trackingSorter::set(aligned_float_vector &arr, int size)
{
	unset();
	this->size = size;
	sorted.resize(size);
	for(int i=0; i<size; ++i)
	{
		sorted[i] = arr[i];
	}
	permuted.reserve(size);
	for(int i=0; i<size; ++i)
	{
		permuted[i] = i;
	}
}

void trackingSorter::unset()
{
	sorted.clear();
	permuted.clear();
	size = -1;
}

void trackingSorter::sort()
{
	generateMaxHeap();
	for(int i=size-1; i>0; --i)
	{
		std::swap(sorted[i], sorted[0]);
		std::swap(permuted[i], permuted[0]);
		versenke(0, i);
	}
}

void trackingSorter::generateMaxHeap()
{
	for(int i = size/2-1; i>=0; --i)
	{
		versenke(i, size);
	}
}

void trackingSorter::versenke(int i, int n)
{
	int ki;
	while(i <= (n>>1)-1)
	{
		ki = ((i+1)<<1)-1;
		
		if(ki+1 <= n-1)
		{
			if(sorted[ki] < sorted[ki+1])
			{
				ki++;
			}
		}
		
		if(sorted[i] < sorted[ki])
		{
			std::swap(sorted[i], sorted[ki]);
			std::swap(permuted[i], permuted[ki]);
			i = ki;
		}
		else
		{
			break;
		}
	}
}

void Bits2Bytes(unsigned char *bits, unsigned char *bytes, int nBytes)
{
	unsigned char tmp;
	for(int i=0; i<nBytes; ++i)
	{
		tmp = 0;
		for(int b=7; b>=0; --b)
		{
			tmp |= (*bits)<<b;
			++bits;
		}
		*bytes = tmp;
		++bytes;
	}	
	
}

void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes)
{
	for(int i=0; i<nBytes; ++i)
	{
		for(int b=7; b>=0; --b)
		{
			*bits = ((*bytes)>>b)&1;
			bits++;
		}
		++bytes;
	}	
}

