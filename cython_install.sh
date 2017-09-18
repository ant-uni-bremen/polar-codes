mkdir cpp_build
cd cpp_build
cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/install ..
make -ja
make install
cd ..
python setup.py build_ext --build-lib cython_build install --prefix=~/install
