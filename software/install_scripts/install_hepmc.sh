#!/bin/bash
cd ../HepMC-2.06.11
mkdir hepmc-build
mkdir hepmc-install
cd hepmc-build
cmake -DCMAKE_INSTALL_PREFIX=../hepmc-install \
      -Dmomentum:STRING=MEV \
      -Dlength:STRING=MM \
      ../
make -j
make test
make install
