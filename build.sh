#!/bin/bash

set +e

echo "Building zlib"
cd zlib
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

echo "Building tinyxmldll"
cd tinyxmldll
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

echo "Building JGTL"
cd JGTL
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

echo "Building Board"
cd Board
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

echo "Building ALE"
cd ale_v0.1
make && cd ..

echo "Builiding HyperNEAT"
cd NE/HyperNEAT/
mkdir -p build && mkdir -p build/release && cd build/release && cmake -i ../../ && make && cd ../../../

set -e