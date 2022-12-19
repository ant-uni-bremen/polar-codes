[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.6325748.svg)](https://doi.org/10.5281/zenodo.6325748) [![Run CI tests](https://github.com/ant-uni-bremen/polar-codes/actions/workflows/run-test.yml/badge.svg)](https://github.com/ant-uni-bremen/polar-codes/actions/workflows/run-test.yml)

# Polar Codes #
This is a library and simulation tool for a Forward Error Correcting (FEC)
scheme named "Polar Codes". A very promising group of codes that have low
encoding and decoding latency with high BLER-performance.

## Installation
This should be straight forward, given that you have a recent compiler (GCC).

You need to clone the repo with the `--recursive` option to ensure that the `pybind11` submodule is checked out as well.
`git clone --recursive`

### Dependencies
Before compiling this project the following packages need to be installed
on your system:

### Debian/Ubuntu
- doxygen
- libcppunit-dev
- libtclap-dev
- libssl-dev
- libfmt-dev
- python3-numpy
- python3-scipy

### The install commands
```
mkdir build
cd build
cmake ..
make -j<cputhreads>
make install
```

## Basic usage
In `build/bin`
```
./pctest  # Run a set of C++ tests
./pcsim   # Run C++ simulations
```

## Python interface usage
With `import pypolar` you can use the Encoders, Decoders, Puncturers and Detectors with Python3. Also, you can use `pypolar.frozen_bits` to get a suitable frozen bit set for polar codes.

A quick example
```python
import numpy as np
import pypolar

frozen_bit_positions = pypolar.frozen_bits(64, 40, 1.0, 'BB')
encoder = pypolar.PolarEncoder(64, frozen_bit_positions)

info_bits = np.random.randint(0, 2, 40, dtype=np.uint8)
info_bytes = np.packbits(info_bits)

codeword_bytes = encoder.encode_vector(info_bytes)
codeword_bits = np.unpackbits(codeword_bytes)

llrs = 1.0 - 2.0 * codeword_bits

decoder = pypolar.PolarDecoder(64, 4, frozen_bit_positions, 'mixed')
hat_bytes = decoder.decode_vector(llrs)

assert np.all(info_bytes == hat_bytes)
```

### Detector interface
We have some CRC polynomials available.
First, we have a CRC-8, CRC-16-CCITTFALSE, and CRC-32 which only work on byte multiples.
Further, we have some 5G NR CRCs available that can operate on any bit size.
- CRC6
- CRC11
- CRC16
- CRC24C

```python
import numpy as np
import pypolar

detector = pypolar.Detector(11, "CRCNR")
data = np.array([1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0], dtype=np.uint8)
packed_data = np.packbits(data)
# NumPy (and probably most software) append 0's to pack bytes. Here [0, 0, 0]
reference = 0x06c8

print(f'0x{reference:03x}')
# We expect packed input with packed_data: 2 byte
# We provide the total number of bits: 13
checksum = detector.calculate(packed_data, data.size)
print(f'0x{checksum:03x}')
```
As seen in the example, we can't infer the actual number of significant bits from the packed vector data size.

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


## Troubleshooting
Sometimes, the installed version is not on the correct path. In case you used `cmake ..` without further arguments, it might help to run:
```
export PYTHONPATH=/usr/local/lib/python3/dist-packages:$PYTHONPATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```
in every terminal you open. Or put this in your `.bashrc` or `.zshrc` or your shell config file. The `/usr/local` part in the lines above must match your `CMAKE_INSTALL_PREFIX`. You can set this to any path with `cmake -DCMAKE_INSTALL_PREFIX=/your/custom/fancy/install/prefix ..`. A more permanent solution needs further consideration because of the interaction between different system components.

## Benchmark
Theoretically, there is `libbenchmark-dev` in Ubuntu 20.04 but we need a newer [benchmark](https://github.com/google/benchmark) version. Thus, we must build and install from source.


## References

This implementation relies on prior publications. We'd like to point out some of them.

* E. Arıkan, "Channel Polarization: A Method for Constructing Capacity-Achieving Codes for Symmetric Binary-Input Memoryless Channels", IEEE Transactions on Information Theory, July 2009, DOI: [10.1109/TIT.2009.2021379](https://doi.org/10.1109/TIT.2009.2021379)
* P. Giard "High-Speed Decoders for Polar Codes", McGill University, Montreal, Canada, September 2016, DOI: [10.1007/978-3-319-59782-9](https://doi.org/10.1007/978-3-319-59782-9), https://escholarship.mcgill.ca/concern/theses/3r074x791

## License and Referencing
This program is licensed under the GPLv3+ license, see [[COPYING]].

### CRCpp
Some CRCs are calculated with [CRC++](https://github.com/d-bahr/CRCpp).
CRC++ is free to use and provided under a BSD license.
