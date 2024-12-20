# AO-WebGPU-Demo

## Setup

1. run `git submodule update --init --recursive` 
2. run `./build_docker.sh`
3. copy the `gml` source files from [this folder](https://github.com/g-truc/glm/tree/master/glm) into [wasm_webgpu/glm](wasm_webgpu/glm)
4. run `./build.sh`

## Test

1. cd into `tests`
2. run `npm install`
3. run `npm run test:still` or `npm run test:frames`
4. check output folder `tests/output` for the generated image(s)
