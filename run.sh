#!/bin/bash
./build/src/SolanaVerifier --contract tests/"$1"/contract --esbmc ~/CLionProjects/esbmc-7.5/cmake-build-debug/src/esbmc/esbmc