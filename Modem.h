#ifndef MODEM_H
#define MODEM_H

/*class Modem
{
public:
	Modem();
	~Modem();*/
	
	void modulate(float *signal, unsigned char *data, int nBytes);
	void softDemod(float *LLR, float *signal, int length, float R, float EbN0);
/*
};*/

#endif
