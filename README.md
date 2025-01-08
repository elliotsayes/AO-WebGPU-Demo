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

## Example output

```txt
using GPU adapter: Apple M1 Max
...
Got device: <wgpu::Device 0x1>
Creating shader module...
[Asyncify] addSleepTaskOnce Data-Race Validation
Shader module: <wgpu::ShaderModule 0x1>
Creating render pipeline...
Render pipeline: <wgpu::RenderPipeline 0x1>
[Asyncify] addSleepTaskOnce RenderPassEncoderRelease
[Asyncify] addSleepTaskOnce TextureViewRelease
[Asyncify] addSleepTaskOnce CommandEncoderRelease
[Asyncify] addSleepTaskOnce QueueSubmit[1]
[Asyncify] addSleepTaskOnce QueueOnSubmittedWorkDone[1]
[Asyncify] addSleepTaskOnce CommandBufferRelease
[Asyncify] running SleepTasks [
  'Data-Race Validation',
  'RenderPassEncoderRelease',
  'TextureViewRelease',
  'CommandEncoderRelease',
  'QueueSubmit[1]',
  'CommandBufferRelease',
  'QueueOnSubmittedWorkDone[1]'
]
Kernel DRF status { fs_main: true, vs_main: true }
All Kernels Data-Race Free? true
[Asyncify] running SleepTasks []
[Asyncify] addSleepTaskOnce Data-Race Validation
[Asyncify] addSleepTaskOnce QueueSubmit[1]
[Asyncify] addSleepTaskOnce QueueOnSubmittedWorkDone[1]
[Asyncify] addSleepTaskOnce CommandEncoderRelease
[Asyncify] addSleepTaskOnce CommandBufferRelease
[Asyncify] addSleepTaskOnce BufferMapAsync
[Asyncify] running SleepTasks [
  'Data-Race Validation',
  'QueueSubmit[1]',
  'CommandEncoderRelease',
  'CommandBufferRelease',
  'BufferMapAsync',
  'QueueOnSubmittedWorkDone[1]'
]
Kernel DRF status { fs_main: true, vs_main: true }
All Kernels Data-Race Free? true
PNG size: 0x40589d0B
...
```

## Diffs

- [ao](https://github.com/permaweb/ao/compare/main...elliotsayes:ao:webgpu)
- [aos](https://github.com/permaweb/aos/compare/main...elliotsayes:aos:webgpu)
- [emscripten](https://github.com/emscripten-core/emscripten/compare/main...elliotsayes:emscripten:webgpu-sync)
