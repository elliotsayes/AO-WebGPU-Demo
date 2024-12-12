#!/bin/bash
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

WEBGPU_DIR="${SCRIPT_DIR}/wasm_webgpu"
PROCESS_DIR="${SCRIPT_DIR}/aos/process"
LIBS_DIR="${PROCESS_DIR}/libs"

rm -rf ${LIBS_DIR}
mkdir -p ${LIBS_DIR}

# AO_IMAGE="p3rmaw3b/ao:0.1.3"
AO_IMAGE="p3rmaw3b/ao:dev"
AO_IMAGE_SYNC="p3rmaw3b/ao:webgpu-sync"

EMXX_CFLAGS="-s SUPPORT_LONGJMP=1 -s USE_WEBGPU=1"
EMXX_CFLAGS_LUA="-s SUPPORT_LONGJMP=1 -s USE_WEBGPU=1 /lua-5.3.4/src/liblua.a -I/lua-5.3.4/src"

# Build wasm_webgpu into a library with emscripten
docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
		"cd /wasm_webgpu && emcc -c helloboat.cpp -o helloboat.o ${EMXX_CFLAGS} && emar r helloboat.a helloboat.o"

# Build lsokoldemo into a library with emscripten
docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
		"cd /wasm_webgpu && emcc -c lsokoldemo.c -o lsokoldemo.o ${EMXX_CFLAGS_LUA} && emar rcs lsokoldemo.a lsokoldemo.o"

# Fix permissions
sudo chmod -R 777 ${WEBGPU_DIR}

# # Copy to the libs directory
cp ${WEBGPU_DIR}/helloboat.a $LIBS_DIR/helloboat.a
cp ${WEBGPU_DIR}/lsokoldemo.a $LIBS_DIR/lsokoldemo.a

# Copy config.yml to the process directory
cp ${SCRIPT_DIR}/config.yml ${PROCESS_DIR}/config.yml

# Build the process module
cd ${PROCESS_DIR}
docker run -e DEBUG=1 --platform linux/amd64 -v ./:/src ${AO_IMAGE_SYNC} ao-build-module
# Copy the sync glue js
cp ${PROCESS_DIR}/process.js ${SCRIPT_DIR}/ao/loader/src/formats/emscripten-webgpu-sync.cjs
docker run -e DEBUG=1 --platform linux/amd64 -v ./:/src ${AO_IMAGE} ao-build-module
# Copy the unsafe glue js
cp ${PROCESS_DIR}/process.js ${SCRIPT_DIR}/ao/loader/src/formats/emscripten-webgpu-unsafe.cjs

cd ${SCRIPT_DIR}/ao/loader
npm run format-file src/formats/emscripten-webgpu-sync.cjs
npm run format-file src/formats/emscripten-webgpu-unsafe.cjs
npm run patch:webgpu
npm run build

# Copy the process module to the tests directory
cp ${PROCESS_DIR}/process.wasm ${SCRIPT_DIR}/tests/process.wasm
