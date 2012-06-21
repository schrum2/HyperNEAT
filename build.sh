#!/bin/bash

cd zlib
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

cd tinyxmldll
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

cd JGTL
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

cd Board
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

cd NE/HyperNEAT/
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../