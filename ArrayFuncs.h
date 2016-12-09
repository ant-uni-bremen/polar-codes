#ifndef ARRAYFUNCS_H
#define ARRAYFUNCS_H

#include <vector>
#include "AlignedAllocator.h"

struct trackingSorter
{
	std::vector<float> sorted;
	std::vector<int> permuted;
	int size;
	trackingSorter();
	trackingSorter(std::vector<float> &arr);
	~trackingSorter();
	void set(std::vector<float> &arr);
	void set(aligned_float_vector &arr, int size);
	void unset();
	void sort();
	
	void generateMaxHeap();
	void versenke(int i, int n);
};

void Bits2Bytes(unsigned char *bits, unsigned char *bytes, int nBytes);
void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes);

#endif

