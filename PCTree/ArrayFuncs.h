/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef ARRAYFUNCS_H
#define ARRAYFUNCS_H

#include <QVector>

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
	float *data;
	QVector<int> permuted;
	int size;
	trackingSorter();
	trackingSorter(QVector<float> &arr);
	~trackingSorter();
	void set(QVector<float> &arr);
	void set(QVector<float> &arr, int size);
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

#endif
