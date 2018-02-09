#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include <polarcode/construction/bhattacharrya.h>

using namespace std;

struct CodingScheme {
	unsigned int blockLength, infoLength;
	vector<unsigned> frozenBits;
	bool systematic;
	float designSnr;
};

vector<CodingScheme> createRegistry() {
	vector<CodingScheme> reg;
	CodingScheme scheme = {16384, 8192, {}, true, 0.0};
	float dSnr;

	PolarCode::Construction::Bhattacharrya *constructor =
			new PolarCode::Construction::Bhattacharrya();

	reg.clear();

	constructor->setBlockLength(scheme.blockLength);
	constructor->setInformationLength(scheme.infoLength);


	//-1.59 dB
	dSnr = -1.59;
	constructor->setParameterByDesignSNR(dSnr);
	scheme.frozenBits = constructor->construct();
	scheme.designSnr = dSnr;
	reg.push_back(scheme);

	//0.0 dB
	dSnr = 0.0;
	constructor->setParameterByDesignSNR(dSnr);
	scheme.frozenBits = constructor->construct();
	scheme.designSnr = dSnr;
	reg.push_back(scheme);

	//1.0 dB
	dSnr = 1.0;
	constructor->setParameterByDesignSNR(dSnr);
	scheme.frozenBits = constructor->construct();
	scheme.designSnr = dSnr;
	reg.push_back(scheme);


/*	//Rate 1/8
	scheme.infoLength = scheme.blockLength / 8;
	constructor->setBlockLength(scheme.blockLength);
	constructor->setInformationLength(scheme.infoLength);
	scheme.frozenBits = constructor->construct();
	reg.push_back(scheme);

	//Rate 1/4
	scheme.infoLength = scheme.blockLength / 4;
	constructor->setBlockLength(scheme.blockLength);
	constructor->setInformationLength(scheme.infoLength);
	scheme.frozenBits = constructor->construct();
	reg.push_back(scheme);

	//Rate 1/2
	scheme.infoLength = scheme.blockLength / 2;
	constructor->setBlockLength(scheme.blockLength);
	constructor->setInformationLength(scheme.infoLength);
	scheme.frozenBits = constructor->construct();
	reg.push_back(scheme);
*/
	return reg;
}

void splitFrozenBits(vector<unsigned> &source,
		unsigned subBlockLength,
		vector<unsigned> &left,
		vector<unsigned> &right) {
	left.resize(source.size());
	right.resize(source.size());
	unsigned leftCounter = 0, rightCounter = 0;

	for(unsigned value : source) {
		if(value < subBlockLength) {
			left[leftCounter++] = value;
		} else {
			right[rightCounter++] = value-subBlockLength;
		}
	}
	left.resize(leftCounter);
	right.resize(rightCounter);
}

enum decoderSide {
	DECTOP,
	DECLEFT,
	DECRIGHT
};

string writeDecoder(unsigned length, vector<unsigned> frozenBits, unsigned bitAddress, decoderSide side) {
	string ret;
	string sLength = to_string(length);
	string sHalfLength = to_string(length/2);

	unsigned frozenLength = frozenBits.size();
	unsigned stage = log2(length);
	unsigned rightBitAddress = bitAddress+((length/2+31)/32);
	string sstage = to_string(stage);

	string inputLlr, lowerLlr;
	string outputBits, leftBits, rightBits;

	//LLR memory
	if(side == DECTOP) {
		inputLlr = "vLlrPtr";
	} else {
		inputLlr = "mLlr[" + to_string(stage) + "]";
	}

	lowerLlr = "mLlr[" + to_string(stage-1) + "]";

	//Bit memory
	switch (side) {
	case DECTOP:
		outputBits = "vBitPtr";
		break;
	case DECLEFT:
		if(length < 32) {
			outputBits = "&mBitL[" + sstage + "]";
		} else {
			outputBits = "vBitPtr+" + to_string(bitAddress);
		}
		break;
	case DECRIGHT:
		if(length < 32) {
			outputBits = "&mBitR[" + sstage + "]";
		} else {
			outputBits = "vBitPtr+" + to_string(bitAddress);
		}
		break;
	default:
		exit(1);
	}

	if(length <= 32) {
		leftBits = "&mBitL[" + to_string(stage-1) + "]";
		rightBits = "&mBitR[" + to_string(stage-1) + "]";
	} else {
		leftBits = "vBitPtr+" + to_string(bitAddress);
		rightBits = "vBitPtr+" + to_string(rightBitAddress);
	}


	//Final decoders

	//Rate 0
	if(frozenLength == length) {
		ret = "	RateZeroDecode<" + sLength + ">(" + outputBits + ");\n";
		return ret;
	}

	//Rate 1
	if(frozenLength == 0) {
		ret = "	RateOneDecode<" + sLength + ">(" + inputLlr + ", " + outputBits + ");\n";
		return ret;
	}

	//Repetition
	if(frozenLength == length-1) {
		ret = "	RepetitionDecode<" + sLength + ">(" + inputLlr + ", " + outputBits + ");\n";
		return ret;
	}

	//SPC
	if(frozenLength == 1) {
		ret = "	SpcDecode<" + sLength + ">(" + inputLlr + ", " + outputBits + ");\n";
		return ret;
	}

	//Search for child code combinations
	vector<unsigned> leftFrozen, rightFrozen;
	splitFrozenBits(frozenBits, length/2, leftFrozen, rightFrozen);

	//Left: Rate 0, Right: SPC
	if(leftFrozen.size() == length/2 && rightFrozen.size() == 1) {
		ret = "	ZeroSpcDecode<" + sLength + ">(" + inputLlr + ", " + outputBits + ");\n";
		return ret;
	}

	//Left Rate 0, Right: Rate 1
	if(length <= 32) {
		if(leftFrozen.size() == length/2 && rightFrozen.size() == 0) {
			ret = "	ZeroOneDecodeShort<" + sLength + ">(" + inputLlr + ", " + outputBits + ");\n";
			return ret;
		}
	}

	//Left: Rate 0, Right: any
	if(leftFrozen.size() == length/2) {
		ret += "	G_function_0R<" + sHalfLength + ">(" + inputLlr + ", " + lowerLlr + ");\n";
		ret += writeDecoder(length/2, rightFrozen, rightBitAddress, DECRIGHT);
		if(length <= 32) {
			ret += "	Combine_0RShort<" + sHalfLength + ">(" + outputBits + ", " + rightBits + ");\n";
		} else {
			ret += "	Combine_0R<" + sHalfLength + ">(" + outputBits + ");\n";
		}
		return ret;
	}

	//Left: any, Right: Rate 1
	if(rightFrozen.size() == 0) {
		ret = "	F_function<" + sHalfLength + ">(" + inputLlr + ", " + lowerLlr + ");\n";
		ret += writeDecoder(length/2, leftFrozen, bitAddress, DECLEFT);
		if(length <= 32) {
			ret += "	simplifiedRightRateOneDecodeShort<" + sHalfLength + ">(" + inputLlr + ", " + leftBits + ", " + outputBits + ");\n";
		} else {
			ret += "	simplifiedRightRateOneDecode<" + sHalfLength + ">(" + inputLlr + ", " + outputBits + ");\n";
		}
		return ret;
	}



	//Left: any, Right: any
	ret = "	F_function<" + sHalfLength + ">(" + inputLlr + ", " + lowerLlr + ");\n";
	ret += writeDecoder(length/2, leftFrozen, bitAddress, DECLEFT);
	ret += "	G_function<" + sHalfLength + ">(" + inputLlr + ", " + lowerLlr + ", " + leftBits + ");\n";
	ret += writeDecoder(length/2, rightFrozen, rightBitAddress, DECRIGHT);
	if(length <= 32) {
		ret += "	CombineSoftBits<" + sHalfLength + ">(" + leftBits + ", " + rightBits + ", " + outputBits + ");\n";
	} else {
		ret += "	CombineSoftInPlace<" + sHalfLength + ">(" + outputBits + ");\n";
	}
	return ret;
}

int main(int argc, char** argv) {
	cout << "This is the factory for fixed decoder creation." << endl;
	if(argc != 2) return 1;

	ofstream file(argv[1]);
	if(!file.is_open()) {
		cout << "File " << argv[1] << " can't been created." << endl;
		return 2;
	}

	std::vector<CodingScheme> registry = createRegistry();

	//Write header

	file << R"TREWQ(#include <polarcode/decoding/avx2_templates.txx>
#include <polarcode/decoding/fixed_avx2_char.h>
#include <array>
#include <immintrin.h>

namespace PolarCode {
namespace Decoding {

std::vector<CodingScheme> codeRegistry = {
)TREWQ";


	//Define the code registry
	for(unsigned i = 0; i < registry.size(); ++i) {
		file << "	{" << registry[i].blockLength << ", "
			 << registry[i].infoLength << ", {";
		for(unsigned j = 0; j < registry[i].frozenBits.size(); ++j) {
			file << registry[i].frozenBits[j];
			if(j+1 < registry[i].frozenBits.size()) {
				file << ",";
			}
		}
		file << "}, " << (registry[i].systematic?"true":"false") << ", " << registry[i].designSnr << "}";
		if(i+1 < registry.size())
			file << ",";
		file << endl;
	}
	file << "};" << endl
		 << endl
		 << "namespace FixedDecoding {" << endl
		 << endl;


	//Declare the decoders

	for(unsigned i = 0; i < registry.size(); ++i) {
		file << "class Fix_" << i << " : public FixedDecoder {" << endl
			 << "	std::array<__m256i, 5> mBitL, mBitR;" << endl
			 << "	std::array<__m256i*, " << (log2(registry[i].blockLength)-1) << "> mLlr;" << endl
			 << endl
			 << "public:" << endl
			 << "	Fix_" << i << "();" << endl
			 << "	~Fix_" << i << "();" << endl
			 << "	void decode(void* LlrIn, void* BitsOut);" << endl
			 << "};" << endl
			 << endl;
	}


	//Define the functions

	for(unsigned i = 0; i < registry.size(); ++i) {
		//Constructor
		file << "Fix_" << i << "::Fix_" << i << "() {" << endl
			 << "	for(unsigned i=0; i<" << (log2(registry[i].blockLength)) << "; ++i) {" << endl
			 << "		mLlr[i] = (__m256i*)_mm_malloc(std::max(1<<i, 32), 32);" << endl
			 << "	}" << endl
			 << "}" << endl
			 << endl;

		//Destructor
		file << "Fix_" << i << "::~Fix_" << i << "() {" << endl
			 << "	for(unsigned i=0; i<" << (log2(registry[i].blockLength)) << "; ++i) {" << endl
			 << "		_mm_free(mLlr[i]);" << endl
			 << "	}" << endl
			 << "}" << endl
			 << endl;

		//Decoder
		string func = writeDecoder(registry[i].blockLength, registry[i].frozenBits, 0, DECTOP);
		file << "void Fix_" << i << "::decode(void* LlrIn, void* BitsOut) {" << endl
			 << "	__m256i *vLlrPtr = (__m256i*)LlrIn;" << endl
			 << "	__m256i *vBitPtr = (__m256i*)BitsOut;" << endl
			 << endl
			 << func << endl
			 << "}" << endl << endl;
	}



	//Function to create a decoder object by scheme number

	file << "FixedDecoder* createFixedDecoder(unsigned int scheme) {\n\tswitch(scheme) {\n";

	for(unsigned i = 0; i < registry.size(); ++i) {
		file << "		case " << i << ": return new Fix_" << i << "();\n";
	}

	file << "		default: return nullptr;" << endl
		 << "	}" << endl
		 << "}" << endl
		 << endl

		// Footer
		 << "}// namespace FixedDecoding" << endl
		 << "}// namespace Decoding" << endl
		 << "}// namespace PolarCode" << endl;

	file.close();

	return 0;
}
