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
//	quicksort(0, size-1);
}

void trackingSorter::stableSort()
{
	float x;
	int j,y;
	for(int i=1; i<size; ++i)
	{
		x = sorted[i];
		y = permuted[i];
		j = i-1;
		while(j>=0 && sorted[j] > x)
		{
			sorted[j+1] = sorted[j];
			permuted[j+1] = permuted[j];
			j--;
		}
		sorted[j+1] = x;
		permuted[j+1] = y;
	}
}

void trackingSorter::stableSortDescending()
{
	float x;
	int j,y;
	for(int i=1; i<size; ++i)
	{
		x = sorted[i];
		y = permuted[i];
		j = i-1;
		while(j>=0 && sorted[j] < x)
		{
			sorted[j+1] = sorted[j];
			permuted[j+1] = permuted[j];
			j--;
		}
		sorted[j+1] = x;
		permuted[j+1] = y;
	}
}

void trackingSorter::partialSort(int n)
{
	//partialQuicksort(0, size-1, n);
	for(int i=0; i<n; ++i)
	{
		int index = i;
		for(int j=i+1; j<size; ++j)
		{
			if(sorted[j] < sorted[index])
			{
				index = j;
			}
		}
		std::swap(sorted[i], sorted[index]);
		std::swap(permuted[i], permuted[index]);
	}
}

void trackingSorter::simplePartialSort(float *data, int size, int n)
{
	sorted.resize(n);
	permuted.resize(n);
	this->size = n;
	
	for(int i=0; i<n; ++i)
	{
		int index = i;
		for(int j=i+1; j<size; ++j)
		{
			if(data[j] < data[index])
			{
				index = j;
			}
		}
		std::swap(data[i], data[index]);
		sorted[i] = data[i];
		permuted[i] = index;
	}
}


void trackingSorter::partialSortDescending(int n)
{
	partialQuicksortDescending(0, size-1, n);
	for(int i=0; i<n; ++i)
	{
		int index = i;
		for(int j=i+1; j<size; ++j)
		{
			if(sorted[j] > sorted[index])
			{
				index = j;
			}
		}
		std::swap(sorted[i], sorted[index]);
		std::swap(permuted[i], permuted[index]);
	}
}


void trackingSorter::sortDescending()
{
/*	float x;
	int j,y;
	for(int i=1; i<size; ++i)
	{
		x = sorted[i];
		y = permuted[i];
		j = i-1;
		while(j>=0 && sorted[j] < x)
		{
			sorted[j+1] = sorted[j];
			permuted[j+1] = permuted[j];
			j--;
		}
		sorted[j+1] = x;
		permuted[j+1] = y;
	}*/
	quicksortDescending(0, size-1);
}

void trackingSorter::quicksort(int lo, int hi)
{
	if(lo<hi)
	{
		int p = partition(lo, hi);
		quicksort(lo, p);
		quicksort(p+1, hi);
	}
}

void trackingSorter::quicksortDescending(int lo, int hi)
{
	if(lo<hi)
	{
		int p = partitionDescending(lo, hi);
		quicksortDescending(lo, p);
		quicksortDescending(p+1, hi);
	}
}

void trackingSorter::partialQuicksort(int lo, int hi, int size)
{
	if(lo < hi)
	{
		int p = partition(lo, hi);
		partialQuicksort(lo, p, size);
		if(p<size-1)
		{
			partialQuicksort(p+1, hi, size);
		}
	}
}

void trackingSorter::partialQuicksortDescending(int lo, int hi, int size)
{
	if(lo < hi)
	{
		int p = partitionDescending(lo, hi);
		partialQuicksortDescending(lo, p, size);
		if(p<size-1)
		{
			partialQuicksortDescending(p+1, hi, size);
		}
	}
}

int trackingSorter::partition(int lo, int hi)
{
	float piv = sorted[lo];
	int i = lo - 1;
	int j = hi + 1;
	for(;;)
	{
		do{
			++i;
		}while(sorted[i] < piv);
		
		do{
			--j;
		}while(sorted[j] > piv);
		
		if(i>=j) return j;
		
		std::swap(sorted[i], sorted[j]);
		std::swap(permuted[i], permuted[j]);
	}
}

int trackingSorter::partitionDescending(int lo, int hi)
{
	float piv = sorted[lo];
	int i = lo - 1;
	int j = hi + 1;
	for(;;)
	{
		do{
			++i;
		}while(sorted[i] > piv);
		
		do{
			--j;
		}while(sorted[j] < piv);
		
		if(i>=j) return j;
		
		std::swap(sorted[i], sorted[j]);
		std::swap(permuted[i], permuted[j]);
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
		tmp = (*bits++)<<7;
		tmp |= (*bits++)<<6;
		tmp |= (*bits++)<<5;
		tmp |= (*bits++)<<4;
		tmp |= (*bits++)<<3;
		tmp |= (*bits++)<<2;
		tmp |= (*bits++)<<1;
		tmp |= *bits++;
		*bytes = tmp;
		++bytes;
	}	
}

void Bits2Bytes(std::vector<bool> &bits, unsigned char *bytes, int nBytes)
{
	unsigned char tmp;
	int j=0;
	for(int i=0; i<nBytes; ++i)
	{
		tmp = bits[j++]<<7;
		tmp |= bits[j++]<<6;
		tmp |= bits[j++]<<5;
		tmp |= bits[j++]<<4;
		tmp |= bits[j++]<<3;
		tmp |= bits[j++]<<2;
		tmp |= bits[j++]<<1;
		tmp |= bits[j++];
		*bytes = tmp;
		++bytes;
	}
}


void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes)
{
	for(int i=0; i<nBytes; ++i)
	{
		*bits++ = ((*bytes)>>7)&1;
		*bits++ = ((*bytes)>>6)&1;
		*bits++ = ((*bytes)>>5)&1;
		*bits++ = ((*bytes)>>4)&1;
		*bits++ = ((*bytes)>>3)&1;
		*bits++ = ((*bytes)>>2)&1;
		*bits++ = ((*bytes)>>1)&1;
		*bits++ = ( *bytes    )&1;
		++bytes;
	}	
}

