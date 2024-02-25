#!/bin/bash
./cmake-build-debug/src/SolanaVerifier --contract tests/"$1"/contract --target process_instruction --esbmc ~/CLionProjects/esbmc-7.5/cmake-build-debug/src/esbmc/esbmc