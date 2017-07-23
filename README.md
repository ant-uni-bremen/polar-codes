## README ##
This is a library and simulation tool for a Forward Error Correcting (FEC)
scheme named "Polar Codes". A very promising group of codes that have low
encoding and decoding latency with high BLER-performance.


# Dependencies #
Before compiling this project the following packages need to be installed
on your system:

- doxygen
- libcppunit-dev
- libtclap-dev

# Basic usage #
    cmake .
    make
    cd bin
    ./pcsim
    
To do: Maybe this can be refined to install the simulation into the system.
Then, the simulation results can be written into a directory of choice.

# Library description #
The library is split into four independent modules:

- An encoder,
- a decoder,
- error detection for list decoding and
- a code constructor

Their respective base classes are purely virtual, so every algorithm
implementation can be called via the base object's functions. This allows for
easy testing as you can throw any algorithm at a general performance tester
without having to modify the testers code in any way.