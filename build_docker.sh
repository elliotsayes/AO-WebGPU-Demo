#!/bin/zsh

ROOT_DIR=$(pwd)

# cd emsdk/docker
# make build version=sdk-main-64bit

cd $ROOT_DIR

# clean up the last emscripten source directory
rm -rf ao/dev-cli/container/src/emscripten/src
mkdir -p ao/dev-cli/container/src/emscripten/src

# use git to find changed files since tag 3.1.73 and copy them over
cd emscripten
git diff 3.1.73 --name-only | xargs -I {} cp {} ../ao/dev-cli/container/src/emscripten/src
cd $ROOT_DIR

cd ao/dev-cli/container
./build.sh
