#include <polarcode/encoding/encoder.h>
#include <polarcode/decoding/decoder.h>
#include <polarcode/construction/constructor.h>
#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {

/**
 * @brief The PolarCoder class ultimately merges all algorithms
 */
class PolarCoder {
	Encoding::Encoder* mEncoder;
	Decoding::Decoder* mDecoder;

public:
	/*!
	 * \brief Create an object that collects matching encoder and decoder for a specific Polar Code.
	 * \param encoder An encoder that has been initialized. Ownership is taken.
	 * \param decoder A  decoder that has been initialized. Ownership is taken.
	 */
	PolarCoder(Encoding::Encoder* encoder, Decoding::Decoder* decoder);
	~PolarCoder();
};




}
