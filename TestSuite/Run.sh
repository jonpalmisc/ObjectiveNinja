#!/usr/bin/env sh

for f in ./TestSuite/Binaries/*; do
    JSON_PATH=./TestSuite/Oracle/$(basename $f).json

    ./build/DumpInfo $f > ./TestSuite/Oracle/$(basename $f).json
    echo ${JSON_PATH}
done
