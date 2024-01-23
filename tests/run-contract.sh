#!/bin/bash

cd ..
cmake -S . -B build/ || exit 1
cmake --build build/ || exit 1

./build/src/SolanaVerifier --contract "$1" --target "$2"