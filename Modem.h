#ifndef MODEM_H
#define MODEM_H

#include "AlignedAllocator.h"

void modulateAndDistort(aligned_float_vector &signal, aligned_float_vector &data, int size, float factor);
void softDemod(aligned_float_vector &LLR, aligned_float_vector &signal, int size, float R, float EbN0);


#endif
