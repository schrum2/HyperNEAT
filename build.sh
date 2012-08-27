#!/bin/bash

# WARNING: This is not a makefile! It is provided only to give a sense of the commands
# necessary to build this whole project. 

set -e

if [[ ! -f zlib/out/libz.so || ! -f zlib/out/libzlib.a ]]
then
    echo "Building zlib"
    cd zlib
    mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../
fi

if [[ ! -f tinyxmldll/out/libtinyxmlplus.so || ! -f tinyxmldll/out/libtinyxmlpluslib.a ]]
then
    echo "Building tinyxmldll"
    cd tinyxmldll
    mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../
fi

if [[ ! -f JGTL/out/ColorTests || ! -f JGTL/out/MapTests || ! -f JGTL/out/RandomTests || ! -f JGTL/out/TreeTests || ! -f JGTL/out/VariantTests ]]
then
    echo "Building JGTL"
    cd JGTL
    mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../
fi

if [[ ! -f Board/out/libboard.a ]]
then
    echo "Building Board"
    cd Board
    mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../
fi

if [[ ! -f ale/ale || ! -f ale/libale.so ]]
then
    echo "Building ALE"
    cd ale
    make -f makefile.unix
    cd ..
fi

if [[ ! -f NE/HyperNEAT/out/atari_evaluate || ! -f NE/HyperNEAT/out/atari_generate || ! -f NE/HyperNEAT/out/atari_visualize ]]
then
    echo "Builiding HyperNEAT"
    cd NE/HyperNEAT/
    mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../
fi

set +e