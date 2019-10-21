#ifndef PC_PUNCTURER_H
#define PC_PUNCTURER_H

#include <cstddef>
#include <vector>
#include <algorithm>
#include <iostream>

namespace PolarCode {

/*!
 * \brief Round up to the next higher power of 2
 * Source: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 * BE CAREFUL! This algorithm doesn't care for size_t > 32bit!
 */
size_t round_up_power_of_two(size_t value);

std::vector<unsigned> inverse_set_difference(size_t blockLength,
                                             std::vector<unsigned> positions);

/*!
 * \brief The Puncturer class
 *
 * For flexible code lengths, we need puncturing.
 */
class Puncturer {

protected:
	size_t mBlockLength; // Punctured block length
	size_t mParentBlockLength; // parent code block length
	std::vector<unsigned> mOutputPositions; // The set of frozen bits.


public:
	Puncturer(const size_t blockLength,
	          const std::vector<unsigned> frozenBitPositions);

	virtual ~Puncturer();

	/*!
	 * \brief Punctured block length
	 * \return Number of elements in punctured vector
	 */
	size_t blockLength(){return mBlockLength;}

	/*!
	 * \brief Parent block length
	 * \return Number of elements in input code vector
	 */
	size_t parentBlockLength(){return mParentBlockLength;}

	/*!
	 * \brief Positions in the input that are copied to the output
	 * \return Order vector with all positions in the input that are present in the output vector
	 */
	std::vector<unsigned> blockOutputPositions(){return mOutputPositions;}

	/*!
	 * \brief Copy elements from pInput at blockOutputPositions to pOutput
	 * \return This is a pointer interface. Beware of the implications!
	 */
	template<typename T>
	void puncture(T *pOutput, const T *pInput){
		// unsigned char* pO = static_cast<unsigned char*>(pOutput);
		// const unsigned char* pI = static_cast<const unsigned char*>(pInput);
		for(auto p : mOutputPositions){
			*pOutput++ = pInput[p];
		}
	}

	/*!
	 * \brief Copy bit elements from pInput at blockOutputPositions to pOutput
	 * \return This is a pointer interface. Beware of the implications!
	 */
	void puncturePacked(unsigned char *pOutput, const unsigned char *pInput);

	/*!
	 * \brief Copy elements from pInput to pOutput at blockOutputPositions all other positions are set to 0.
	 * \return This is a pointer interface. Beware of the implications!
	 */
	template<typename T>
	void depuncture(T *pOutput, const T *pInput){
		std::fill(pOutput, pOutput + mParentBlockLength, 0);
		for(auto p : mOutputPositions){
			pOutput[p] = *pInput++;
		}
	};

};

}//namespace PolarCode

#endif
