#!/bin/bash
cmake -S . -B build/ || exit 1
cmake --build build/ || exit 1