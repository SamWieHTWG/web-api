#!/bin/bash

export LD_LIBRARY_PATH=$(pwd)/../../lib/x86_64

mkdir -p ./listen
cp -r ../../examples/example2-full-demo/cfg/. ./listen/

mkdir -p ./prg
cp -r ../../examples/components/example-nc-programs/. ./prg/

mkdir -p ./error
cp -r ../../components/error/. ./error/

chmod +x CncSDKDemo
./CncSDKDemo
