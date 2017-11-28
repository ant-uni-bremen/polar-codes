#!/bin/bash
cd ..
mkdir -p polar-codes-release
cd polar-codes-release
/usr/bin/cmake ../polar-codes '-GCodeBlocks - Unix Makefiles' -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_CXX_COMPILER:STRING=/usr/bin/g++ -DCMAKE_C_COMPILER:STRING=/usr/bin/gcc -DCMAKE_PREFIX_PATH:STRING=/usr
make -j4
