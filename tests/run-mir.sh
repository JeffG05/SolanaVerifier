#!/bin/bash

cd ..
cmake -S . -B build/ || exit 1
cmake --build build/ || exit 1

./build/src/SolanaVerifier --mir "$1" --target "$2"