#!/usr/bin/env bash
set -euo pipefail

# prefer newer gcc for full c++23 support (<print>, etc.)
for v in 15 14; do
    if command -v "g++-$v" >/dev/null 2>&1; then
        export CXX="g++-$v"
        export CC="gcc-$v"
        break
    fi
done

cmake -B build
cmake --build build -j
cp build/microtcp ./microtcp
