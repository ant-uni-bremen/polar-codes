#ifndef MODEM_H
#define MODEM_H

#include <vector>

void modulate(std::vector<float> &signal, std::vector<bool> &data);
void softDemod(std::vector<float> &LLR, std::vector<float> &signal, float R, float EbN0);


#endif
