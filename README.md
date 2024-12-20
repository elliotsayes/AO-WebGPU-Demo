# AO-SQLite

## Setup

1. run `./build_docker.sh`
2. Copy the `gml` source files from [this folder](https://github.com/g-truc/glm/tree/master/glm) into [wasm_webgpu/glm](wasm_webgpu/glm)
3. run `./build.sh`

## Test

1. cd `tests`
2. `npm install`
3. `npm run test:still` or `npm run test:frames`
4. Check output folder `tests/output` for the generated image(s)
