#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

SQLLITE_DIR="${SCRIPT_DIR}/sqlite3"
PROCESS_DIR="${SCRIPT_DIR}/aos/process"
LIBS_DIR="${PROCESS_DIR}/libs"

rm -rf ${LIBS_DIR}
mkdir -p ${LIBS_DIR}

AO_IMAGE="p3rmaw3b/ao:0.1.2"

EMXX_CFLAGS="-s MEMORY64=1 -s SUPPORT_LONGJMP=1 /lua-5.3.4/src/liblua.a -I/lua-5.3.4/src"

# Build sqlite3 into a library with emscripten
docker run -v ${SQLLITE_DIR}:/sqlite3 ${AO_IMAGE} sh -c \
		"cd /sqlite3 && emcc -s -c sqlite3.c -o sqlite3.o ${EMXX_CFLAGS} && emar r sqlite3.a sqlite3.o && rm sqlite3.o"

# Build lsqlite3 into a library with emscripten
docker run -v ${SQLLITE_DIR}:/sqlite3 ${AO_IMAGE} sh -c \
		"cd /sqlite3 && emcc -s -c lsqlite3.c -o lsqlite3.o ${EMXX_CFLAGS} && emar rcs lsqlite3.a lsqlite3.o && rm lsqlite3.o"

# Fix permissions
sudo chmod -R 777 ${SQLLITE_DIR}


# # Copy luagraphqlparser to the libs directory
cp ${SQLLITE_DIR}/sqlite3.a $LIBS_DIR/sqlite3.a
cp ${SQLLITE_DIR}/lsqlite3.a $LIBS_DIR/lsqlite3.a


# Copy config.yml to the process directory
cp ${SCRIPT_DIR}/config.yml ${PROCESS_DIR}/config.yml

# Build the process module
cd ${PROCESS_DIR} 
docker run -e DEBUG=1 --platform linux/amd64 -v ./:/src ${AO_IMAGE} ao-build-module

# Copy the process module to the tests directory
cp ${PROCESS_DIR}/process.wasm ${SCRIPT_DIR}/tests/process.wasm
# cp ${PROCESS_DIR}/process.js ${SCRIPT_DIR}/tests/process.js