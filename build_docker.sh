#!/bin/zsh

ROOT_DIR=$(pwd)

# cd emsdk/docker
# make build version=sdk-main-64bit

# cd $ROOT_DIR

# copy over the patched emscripten WebGPU library file
mkdir -p ao/dev-cli/container/src/emscripten
cp -r emscripten/src ao/dev-cli/container/src/emscripten

cd ao/dev-cli/container
./build.sh
