#!/bin/sh

mkdir -p build
g++ -std=c++20 test.cpp -o build/test
./build/test