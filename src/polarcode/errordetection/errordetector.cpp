#include <polarcode/errordetection/errordetector.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/crc32.h>

#include <algorithm>
#include <stdexcept>

namespace PolarCode {
namespace ErrorDetection {

Detector* create(unsigned size, std::string type){
    std::transform(type.begin(), type.end(), type.begin(),
                   [](unsigned char c){return std::tolower(c); } );
    Detector* detector;
    if(type.find("crc") != std::string::npos){
        switch (size) {
        case 0: detector = new Dummy(); break;
        case 8: detector = new CRC8(); break;
        case 32: detector = new CRC32(); break;
        default: detector = new Dummy();
        }
    }
    else if(type.find("cmac") != std::string::npos){
        throw std::logic_error("CMAC INTERFACE NOT IMPLEMENTED");
    }
    else{
        throw std::runtime_error("Unknown Error detector requested!");
    }
    return detector;
}

//The Detector-class is purely virtual.

}//namespace ErrorDetection
}//namespace PolarCode
