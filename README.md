# AO-WebGPU-Demo

https://github.com/user-attachments/assets/f3714ec0-64a4-467a-951d-2109b9fb2ae3

## Setup

1. run `git submodule update --init --recursive` 
2. run `./build_docker.sh`
3. copy the `glm` source files from [this folder](https://github.com/g-truc/glm/tree/master/glm) into [wasm_webgpu/glm](wasm_webgpu/glm)
4. run `./build.sh`

## Test

1. cd into `tests`
2. run `npm install`
3. run `npm run test:still` or `npm run test:frames`
4. check output folder `tests/output` for the generated image(s)

## Diffs:

- [ao](https://github.com/permaweb/ao/compare/main...elliotsayes:ao:webgpu)
- [aos](https://github.com/permaweb/aos/compare/main...elliotsayes:aos:webgpu)
- [emscripten](https://github.com/emscripten-core/emscripten/compare/main...elliotsayes:emscripten:webgpu-sync)
