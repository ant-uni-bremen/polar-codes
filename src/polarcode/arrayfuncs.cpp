/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/arrayfuncs.h>

#include <algorithm>
#include <cassert>
#include <utility>

trackingSorter::trackingSorter() { unset(); }

trackingSorter::trackingSorter(std::vector<double>& arr) { set(arr); }

trackingSorter::~trackingSorter() { unset(); }

void trackingSorter::set(std::vector<double>& arr)
{
    unset();
    size = arr.size();
    data = arr.data();
    permuted.resize(size);
    for (int i = 0; i < size; ++i) {
        permuted[i] = i;
    }
}

void trackingSorter::set(std::vector<double>& arr, int size)
{
    unset();
    this->size = size;
    data = arr.data();
    permuted.resize(size);
    for (int i = 0; i < size; ++i) {
        permuted[i] = i;
    }
}

/*
void trackingSorter::set(aligned_float_vector &arr, int size) {
        unset();
        this->size = size;
        data = arr.data();
        permuted.reserve(size);
        for(int i=0; i<size; ++i) {
                permuted[i] = i;
        }
}*/

void trackingSorter::unset()
{
    data = NULL;
    permuted.clear();
    size = 0;
}

void trackingSorter::sort()
{
    generateMaxHeap();
    for (int i = size - 1; i > 0; --i) {
        std::swap(data[i], data[0]);
        std::swap(permuted[i], permuted[0]);
        versenke(0, i);
    }
    //	quicksort(0, size-1);
}

void trackingSorter::stableSort()
{
    double x;
    int j, y;
    for (int i = 1; i < size; ++i) {
        x = data[i];
        y = permuted[i];
        j = i - 1;
        while (j >= 0 && data[j] > x) {
            data[j + 1] = data[j];
            permuted[j + 1] = permuted[j];
            j--;
        }
        data[j + 1] = x;
        permuted[j + 1] = y;
    }
}

void trackingSorter::stableSortDescending()
{
    double x;
    int j, y;
    for (int i = 1; i < size; ++i) {
        x = data[i];
        y = permuted[i];
        j = i - 1;
        while (j >= 0 && data[j] < x) {
            data[j + 1] = data[j];
            permuted[j + 1] = permuted[j];
            j--;
        }
        data[j + 1] = x;
        permuted[j + 1] = y;
    }
}

void trackingSorter::partialSort(int n)
{
    // partialQuicksort(0, size-1, n);
    for (int i = 0; i < n; ++i) {
        int index = i;
        for (int j = i + 1; j < size; ++j) {
            if (data[j] < data[index]) {
                index = j;
            }
        }
        std::swap(data[i], data[index]);
        std::swap(permuted[i], permuted[index]);
    }
}

void trackingSorter::simplePartialSort(double* data, int size, int n)
{
    permuted.resize(size);
    for (int i = 0; i < size; ++i) {
        permuted[i] = i;
    }

    this->size = n;

    int lim = std::min(size - 1, n);

    for (int i = 0; i < lim; ++i) {
        int index = i;
        for (int j = i + 1; j < size; ++j) {
            if (data[j] < data[index]) {
                index = j;
            }
        }
        std::swap(data[i], data[index]);
        std::swap(permuted[i], permuted[index]);
    }
}

void trackingSorter::simplePartialSortDescending(double* data, int size, int n)
{
    permuted.resize(size);
    for (int i = 0; i < size; ++i) {
        permuted[i] = i;
    }

    this->size = n;

    int lim = std::min(size - 1, n);

    for (int i = 0; i < lim; ++i) {
        int index = i;
        for (int j = i + 1; j < size; ++j) {
            if (data[j] > data[index]) {
                index = j;
            }
        }
        std::swap(data[i], data[index]);
        std::swap(permuted[i], permuted[index]);
    }
}

void trackingSorter::partialSortDescending(int n)
{
    partialQuicksortDescending(0, size - 1, n);
    for (int i = 0; i < n; ++i) {
        int index = i;
        for (int j = i + 1; j < size; ++j) {
            if (data[j] > data[index]) {
                index = j;
            }
        }
        std::swap(data[i], data[index]);
        std::swap(permuted[i], permuted[index]);
    }
}


void trackingSorter::sortDescending() { quicksortDescending(0, size - 1); }

void trackingSorter::quicksort(int lo, int hi)
{
    if (lo < hi) {
        int p = partition(lo, hi);
        quicksort(lo, p);
        quicksort(p + 1, hi);
    }
}

void trackingSorter::quicksortDescending(int lo, int hi)
{
    if (lo < hi) {
        int p = partitionDescending(lo, hi);
        quicksortDescending(lo, p);
        quicksortDescending(p + 1, hi);
    }
}

void trackingSorter::partialQuicksort(int lo, int hi, int size)
{
    if (lo < hi) {
        int p = partition(lo, hi);
        partialQuicksort(lo, p, size);
        if (p < size - 1) {
            partialQuicksort(p + 1, hi, size);
        }
    }
}

void trackingSorter::partialQuicksortDescending(int lo, int hi, int size)
{
    if (lo < hi) {
        int p = partitionDescending(lo, hi);
        partialQuicksortDescending(lo, p, size);
        if (p < size - 1) {
            partialQuicksortDescending(p + 1, hi, size);
        }
    }
}

int trackingSorter::partition(int lo, int hi)
{
    float piv = data[lo];
    int i = lo - 1;
    int j = hi + 1;
    for (;;) {
        do {
            ++i;
        } while (data[i] < piv);

        do {
            --j;
        } while (data[j] > piv);

        if (i >= j)
            return j;

        std::swap(data[i], data[j]);
        std::swap(permuted[i], permuted[j]);
    }
}

int trackingSorter::partitionDescending(int lo, int hi)
{
    float piv = data[lo];
    int i = lo - 1;
    int j = hi + 1;
    for (;;) {
        do {
            ++i;
        } while (data[i] > piv);

        do {
            --j;
        } while (data[j] < piv);

        if (i >= j)
            return j;

        std::swap(data[i], data[j]);
        std::swap(permuted[i], permuted[j]);
    }
}

void trackingSorter::generateMaxHeap()
{
    for (int i = size / 2 - 1; i >= 0; --i) {
        versenke(i, size);
    }
}

void trackingSorter::versenke(int i, int n)
{
    int ki;
    while (i <= (n >> 1) - 1) {
        ki = ((i + 1) << 1) - 1;

        if (ki + 1 <= n - 1) {
            if (data[ki] < data[ki + 1]) {
                ki++;
            }
        }

        if (data[i] < data[ki]) {
            std::swap(data[i], data[ki]);
            std::swap(permuted[i], permuted[ki]);
            i = ki;
        } else {
            break;
        }
    }
}

void Bits2Bytes(unsigned char* bits, unsigned char* bytes, int nBytes)
{
    unsigned char tmp;
    for (int i = 0; i < nBytes; ++i) {
        tmp = (*bits++) << 7;
        tmp |= (*bits++) << 6;
        tmp |= (*bits++) << 5;
        tmp |= (*bits++) << 4;
        tmp |= (*bits++) << 3;
        tmp |= (*bits++) << 2;
        tmp |= (*bits++) << 1;
        tmp |= *bits++;
        *bytes = tmp;
        ++bytes;
    }
}

void Bits2Bytes(std::vector<float>& fbits, unsigned char* bytes, int nBytes)
{
    unsigned int* bits = reinterpret_cast<unsigned int*>(fbits.data());
    unsigned int tmp;
    int j = 0;
    for (int i = 0; i < nBytes; ++i) {
        tmp = bits[j++] >> 24;
        tmp |= bits[j++] >> 25;
        tmp |= bits[j++] >> 26;
        tmp |= bits[j++] >> 27;
        tmp |= bits[j++] >> 28;
        tmp |= bits[j++] >> 29;
        tmp |= bits[j++] >> 30;
        tmp |= bits[j++] >> 31;
        *bytes = static_cast<unsigned char>(tmp);
        ++bytes;
    }
}

void Bits2Bytes(float* fbits, unsigned char* bytes, int nBytes)
{
    unsigned int* bits = reinterpret_cast<unsigned int*>(fbits);
    unsigned int tmp;
    int j = 0;
    for (int i = 0; i < nBytes; ++i) {
        tmp = bits[j++] >> 24;
        tmp |= bits[j++] >> 25;
        tmp |= bits[j++] >> 26;
        tmp |= bits[j++] >> 27;
        tmp |= bits[j++] >> 28;
        tmp |= bits[j++] >> 29;
        tmp |= bits[j++] >> 30;
        tmp |= bits[j++] >> 31;
        *bytes = static_cast<unsigned char>(tmp);
        ++bytes;
    }
}

void Bytes2Bits(unsigned char* bytes, unsigned char* bits, int nBytes)
{
    for (int i = 0; i < nBytes; ++i) {
        *bits++ = ((*bytes) >> 7) & 1;
        *bits++ = ((*bytes) >> 6) & 1;
        *bits++ = ((*bytes) >> 5) & 1;
        *bits++ = ((*bytes) >> 4) & 1;
        *bits++ = ((*bytes) >> 3) & 1;
        *bits++ = ((*bytes) >> 2) & 1;
        *bits++ = ((*bytes) >> 1) & 1;
        *bits++ = (*bytes) & 1;
        ++bytes;
    }
}
