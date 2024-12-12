#!/bin/bash
# set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

WEBGPU_DIR="${SCRIPT_DIR}/wasm_webgpu"
PROCESS_DIR="${SCRIPT_DIR}/aos/process"
LIBS_DIR="${PROCESS_DIR}/libs"

# Cleanup files in the wasm_webgpu directory
rm ${WEBGPU_DIR}/*.o
rm ${WEBGPU_DIR}/*.a

# Cleanup files in the aos process directory
rm ${PROCESS_DIR}/config.yml
rm ${PROCESS_DIR}/libs/*
rm ${PROCESS_DIR}/process.wasm
rm ${PROCESS_DIR}/process.js
