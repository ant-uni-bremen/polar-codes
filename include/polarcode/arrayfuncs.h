#ifndef ARRAYFUNCS_H
#define ARRAYFUNCS_H

#include <vector>

/*!
 * \brief A collection of sorting algorithms with permutation information.
 */
class trackingSorter {
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
	float *data; ///< Pointer to memory that is to be sorted.
	std::vector<int> permuted; ///< Permutation vector.
	int size; ///< Number of elements to be sorted.
	trackingSorter();
	trackingSorter(std::vector<float> &arr);
	~trackingSorter();
	void set(std::vector<float> &arr);
	void set(std::vector<float> &arr, int size);
	void unset();
	void sort();
	void sortDescending();
	void stableSort();
	void stableSortDescending();
	void partialSort(int n);
	void partialSortDescending(int n);
	
	void simplePartialSort(float *data, int size, int n);
	void simplePartialSortDescending(float *data, int size, int n);
};

void Bits2Bytes(std::vector<float> &bits, unsigned char *bytes, int nBytes);
void Bits2Bytes(float* fbits, unsigned char *bytes, int nBytes);

void Bits2Bytes(unsigned char *bits, unsigned char *bytes, int nBytes);
void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes);

#endif

