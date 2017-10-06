mkdir cpp_build
cd cpp_build
cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/install ..
make -j
make install
cd ..
python setup.py install --prefix=~/install
