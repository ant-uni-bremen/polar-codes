#ifndef ARRAYFUNCS_H
#define ARRAYFUNCS_H

struct trackingSorter
{
	float *sorted;
	int *permuted;
	int size;
	trackingSorter();
	trackingSorter(float *arr, int size);
	~trackingSorter();
	void set(float *arr, int size);
	void unset();
	void sort();
	
	void generateMaxHeap();
	void versenke(int i, int n);
};

void Bits2Bytes(unsigned char *bits, unsigned char *bytes, int nBytes);
void Bytes2Bits(unsigned char *bytes, unsigned char *bits, int nBytes);

#endif

