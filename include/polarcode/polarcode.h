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
	Construction::Constructor* mConstructor;

public:
	PolarCoder(Encoding::Encoder*, Decoding::Decoder*,
			   Construction::Constructor*);
	~PolarCoder();
};




}
