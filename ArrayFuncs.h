#ifndef ARRAYFUNCS_H
#define ARRAYFUNCS_H

#include <vector>
#include "AlignedAllocator.h"

class trackingSorter
{
private:
	void generateMaxHeap();
	void versenke(int i, int n);
	void quicksort(int lo, int hi);
	void quicksortDescending(int lo, int hi);
	void partialQuicksort(int lo, int hi, int size);
	void partialQuicksortDescending(int lo, int hi, int size);
	int partition(int lo, int hi);
	int partitionDescending(int lo, int hi);

public:
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
	void sortDescending();
	void stableSort();
	void partialSort(int n);
	void partialSortDescending(int n);
	
};


void Bits2Bytes(unsigned char *bits, unsigned char *bytes, int nBytes);
void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes);

#endif

