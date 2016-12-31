#ifndef MODEM_H
#define MODEM_H

#include <vector>
#include "AlignedAllocator.h"

void modulate(std::vector<float> &signal, aligned_float_vector &data);
void softDemod(std::vector<float> &LLR, std::vector<float> &signal, float R, float EbN0);


#endif
