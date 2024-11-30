#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

WEBGPU_DIR="${SCRIPT_DIR}/wasm_webgpu"
PROCESS_DIR="${SCRIPT_DIR}/aos/process"
LIBS_DIR="${PROCESS_DIR}/libs"

rm -rf ${LIBS_DIR}
mkdir -p ${LIBS_DIR}

AO_IMAGE="p3rmaw3b/ao:0.1.3"

EMXX_CFLAGS="-s MEMORY64=1 -s SUPPORT_LONGJMP=1 -s USE_WEBGPU=1"
EMXX_CFLAGS_LUA="-s MEMORY64=1 -s SUPPORT_LONGJMP=1 -s USE_WEBGPU=1 /lua-5.3.4/src/liblua.a -I/lua-5.3.4/src"

# Build wasm_webgpu into a library with emscripten
docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
		"cd /wasm_webgpu && emcc -s -c lib_webgpu.cpp -o lib_webgpu.o ${EMXX_CFLAGS} && emar r lib_webgpu.a lib_webgpu.o && rm lib_webgpu.o"
docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
		"cd /wasm_webgpu && emcc -s -c lib_webgpu_cpp20.cpp -o lib_webgpu_cpp20.o ${EMXX_CFLAGS} && emar r lib_webgpu_cpp20.a lib_webgpu_cpp20.o && rm lib_webgpu_cpp20.o"
# docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
# 		"cd /wasm_webgpu && emcc -s -c lib_webgpu_dawn.cpp -o lib_webgpu_dawn.o ${EMXX_CFLAGS} && emar r lib_webgpu_dawn.a lib_webgpu_dawn.o && rm lib_webgpu_dawn.o"
# docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
# 		"cd /wasm_webgpu && emcc -s -c sokol_gfx.h -o sokol_gfx.o ${EMXX_CFLAGS} && emar r sokol_gfx.a sokol_gfx.o && rm sokol_gfx.o"

# Build lsokoldemo into a library with emscripten
docker run -v ${WEBGPU_DIR}:/wasm_webgpu ${AO_IMAGE} sh -c \
		"cd /wasm_webgpu && emcc -s -c lsokoldemo.c -o lsokoldemo.o ${EMXX_CFLAGS_LUA} && emar rcs lsokoldemo.a lsokoldemo.o && rm lsokoldemo.o"

# Fix permissions
sudo chmod -R 777 ${WEBGPU_DIR}

# # Copy luagraphqlparser to the libs directory
cp ${WEBGPU_DIR}/lib_webgpu.a $LIBS_DIR/lib_webgpu.a
cp ${WEBGPU_DIR}/lib_webgpu_cpp20.a $LIBS_DIR/lib_webgpu_cpp20.a
# cp ${WEBGPU_DIR}/lib_webgpu_dawn.a $LIBS_DIR/lib_webgpu_dawn.a
# cp ${WEBGPU_DIR}/sokol_gfx.a $LIBS_DIR/sokol_gfx.a
cp ${WEBGPU_DIR}/lsokoldemo.a $LIBS_DIR/lsokoldemo.a

# Copy config.yml to the process directory
cp ${SCRIPT_DIR}/config.yml ${PROCESS_DIR}/config.yml

# Build the process module
cd ${PROCESS_DIR} 
docker run -e DEBUG=1 --platform linux/amd64 -v ./:/src ${AO_IMAGE} ao-build-module

# Copy the process module to the tests directory
cp ${PROCESS_DIR}/process.wasm ${SCRIPT_DIR}/tests/process.wasm

# Setup the glue js
cp ${PROCESS_DIR}/process.js ${SCRIPT_DIR}/ao/loader/src/formats/wasm64-emscripten-webgpu.cjs
cd ${SCRIPT_DIR}/ao/loader
npm run format-file src/formats/wasm64-emscripten-webgpu.cjs
npm run patch:webgpu
npm run build

# Cleanup files in the aos process directory
rm ${PROCESS_DIR}/config.yml
rm ${PROCESS_DIR}/libs/*
