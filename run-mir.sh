#!/bin/bash
./build/src/SolanaVerifier --mir tests/official/"$1"/results/result.mir --hir tests/"$1"/results/result.hir --esbmc ~/CLionProjects/esbmc-7.5/cmake-build-debug/src/esbmc/esbmc