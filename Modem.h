#ifndef MODEM_H
#define MODEM_H

#include "AlignedAllocator.h"

void modulateAndDistort(float *signal, aligned_float_vector &data, int size, float factor);

#endif
