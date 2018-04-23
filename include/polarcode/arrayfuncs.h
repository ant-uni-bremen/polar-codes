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
	double *data; ///< Pointer to memory that is to be sorted.
	std::vector<int> permuted; ///< Permutation vector.
	int size; ///< Number of elements to be sorted.
	trackingSorter();

	/*!
	 * \brief Create and initialize a sorter.
	 * \param arr The vector to be sorted.
	 */
	trackingSorter(std::vector<double> &arr);
	~trackingSorter();

	/*!
	 * \brief Set the vector to be sorted and initialize the permutation vector.
	 * \param arr The vector to be sorted.
	 */
	void set(std::vector<double> &arr);

	/*!
	 * \brief Set a subset of a vector to be sorted and initialize the permutation vector.
	 * \param arr The vector to be sorted.
	 * \param size The number of elements to be taken from the beginning of the vector.
	 */
	void set(std::vector<double> &arr, int size);

	/*!
	 * \brief Clear the contents of this object.
	 */
	void unset();

	/*!
	 * \brief Sort the data in ascending order.
	 */
	void sort();

	/*!
	 * \brief Sort the data in descending order.
	 */
	void sortDescending();

	/*!
	 * \brief Sort ascending and keep original order of already sorted values.
	 */
	void stableSort();

	/*!
	 * \brief Sort descending and keep original order of already sorted values.
	 */
	void stableSortDescending();

	/*!
	 * \brief Find n lowest values of the vector and leave the rest unsorted.
	 * \param n The number of lowest elements to find and sort.
	 */
	void partialSort(int n);

	/*!
	 * \brief Find n highest values of the vector and leave the rest unsorted.
	 * \param n The number of highest elements to find and sort.
	 */
	void partialSortDescending(int n);

	/*!
	 * \brief Quickly find indices of n lowest values.
	 * \param data The vector that will be partialliy sorted.
	 * \param size Size of that vector.
	 * \param n Number of elements to sort.
	 */
	void simplePartialSort(double *data, int size, int n);

	/*!
	 * \brief Quickly find indices of n highest values.
	 * \param data The vector that will be partialliy sorted.
	 * \param size Size of that vector.
	 * \param n Number of elements to sort.
	 */
	void simplePartialSortDescending(double *data, int size, int n);
};

//TODO: Clean up the following mess of partial sorting functions.

template<typename IdxType, typename ValueType>
void simplePartialSortDescending(
		std::vector<IdxType> &Indices,
		std::vector<ValueType> &Values,
		const unsigned int n) {
	const unsigned size = Values.size();
	Indices.resize(size);
	for(unsigned i=0; i<size; ++i) {
		Indices[i] = i;
	}

	const unsigned lim = std::min(size-1, n);

	for(unsigned i=0; i<lim; ++i) {
		unsigned index = i;
		for(unsigned j=i+1; j<size; ++j) {
			if(Values[j] > Values[index]) {
				index = j;
			}
		}
		std::swap(Values[i], Values[index]);
		std::swap(Indices[i], Indices[index]);
	}
}

template<typename IdxType, typename ValueType>
void simplePartialSortDescending(
		std::vector<IdxType> &Indices,
		ValueType *Values,
		const unsigned int size,
		const unsigned int n) {
	Indices.resize(size);
	for(unsigned i=0; i<size; ++i) {
		Indices[i] = i;
	}

	const unsigned lim = std::min(size-1, n);

	for(unsigned i=0; i<lim; ++i) {
		unsigned index = i;
		for(unsigned j=i+1; j<size; ++j) {
			if(Values[j] > Values[index]) {
				index = j;
			}
		}
		std::swap(Values[i], Values[index]);
		std::swap(Indices[i], Indices[index]);
	}
}

template<typename IdxType, typename ValueType>
void simplePartialSortDescending(
		std::vector<IdxType> &Indices,
		std::vector<ValueType> &Values,
		const unsigned int n,
		const unsigned int size) {
	for(unsigned i=0; i<size; ++i) {
		Indices[i] = i;
	}

	const unsigned lim = std::min(size-1, n);

	for(unsigned i=0; i<lim; ++i) {
		unsigned index = i;
		for(unsigned j=i+1; j<size; ++j) {
			if(Values[j] > Values[index]) {
				index = j;
			}
		}
		std::swap(Values[i], Values[index]);
		std::swap(Indices[i], Indices[index]);
	}
}

template<typename IdxType, typename ValueType>
void sortMetrics(
		std::vector<IdxType> &Indices,
		std::vector<ValueType> &Values,
		const unsigned int n,
		const unsigned int size) {
	for(unsigned i=0; i<size; ++i) {
		Indices[i] = i;
	}

	const unsigned lim = std::min(size-1, n);

	for(unsigned i=0; i<lim; ++i) {
		unsigned index = i;
		for(unsigned j=size-1; j>i; --j) {
			if(Values[j] < Values[index]) {
				index = j;
			}
		}
		std::swap(Values[i], Values[index]);
		std::swap(Indices[i], Indices[index]);
	}
}

template<typename IdxType, typename ValueType>
void findWeakLlrs(
		std::vector<IdxType> &Indices,
		ValueType *Values,
		const unsigned int size,
		const unsigned int n) {
	for(unsigned i=0; i<size; ++i) {
		Indices[i] = i;
	}

	const unsigned lim = std::min(size-1, n);

	for(unsigned i=0; i<lim; ++i) {
		unsigned index = i;
		for(unsigned j=i+1; j<size; ++j) {
			if(Values[j] < Values[index]) {
				index = j;
			}
		}
		std::swap(Values[i], Values[index]);
		std::swap(Indices[i], Indices[index]);
	}
}


void Bits2Bytes(std::vector<float> &bits, unsigned char *bytes, int nBytes);
void Bits2Bytes(float* fbits, unsigned char *bytes, int nBytes);

void Bits2Bytes(unsigned char *bits, unsigned char *bytes, int nBytes);
void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes);

#endif

