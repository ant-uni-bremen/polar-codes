# Polar Codes #
This is a library and simulation tool for a Forward Error Correcting (FEC)
scheme named "Polar Codes". A very promising group of codes that have low
encoding and decoding latency with high BLER-performance.


## Dependencies
Before compiling this project the following packages need to be installed
on your system:

### Debian/Ubuntu
- doxygen
- libcppunit-dev
- libtclap-dev

## Basic usage #
    cmake .
    make
    cd bin
    ./pcsim

## Python interface usage
With `import pypolar` you can use the Encoders and Decoders with Python3.
At the moment, this depends on cython and an extra install path.

`python3 setup.py build` <br>
`python3 setup.py install --prefix=[CMAKE_INSTALL_PREFIX]`

## Library description #
The library is split into four independent modules:

- An encoder,
- a decoder,
- error detection for list decoding and
- a code constructor

Their respective base classes are purely virtual, so every algorithm
implementation can be called via the base object's functions. This allows for
easy testing as you can throw any algorithm at a general performance tester
without having to modify the testers code in any way.