#include <emscripten.h>
#include "emscripten.h"
#include "emscripten/html5.h"
#include "emscripten/html5_webgpu.h"
#include "webgpu/webgpu.h"

// #include <stdio.h>
// #define SOKOL_IMPL
// #define SOKOL_WGPU
// #include "sokol_gfx.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

void onQueueWorkDone(WGPUQueueWorkDoneStatus status, void* pUserData) {
    printf("Queued work finished with status: %d\n", status);
}

void commandDemo(WGPUDevice device, WGPUQueue queue) {
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, NULL /* pUserData */);

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = NULL;
    encoderDesc.label = "My command encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = NULL;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    
    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command); // release command buffer once submitted

    wgpuCommandEncoderRelease(encoder); // release encoder after it's finished
}

void textureDemo(WGPUDevice device, WGPUQueue queue) {
        // We keep a target format though, for instance:
    WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm;

    // During initialization
    WGPUTextureDescriptor targetTextureDesc = {};
    targetTextureDesc.nextInChain = NULL;
    targetTextureDesc.label = "Render target";
    targetTextureDesc.dimension = WGPUTextureDimension_2D;
    // Any size works here, this is the equivalent of the window size

    WGPUExtent3D outputSize = {};
    outputSize.width = 800;
    outputSize.height = 600;
    outputSize.depthOrArrayLayers = 1;

    targetTextureDesc.size = outputSize;
    // Use the same format here and in the render pipeline's color target
    targetTextureDesc.format = swapChainFormat;
    // No need for MIP maps
    targetTextureDesc.mipLevelCount = 1;
    // You may set up supersampling here
    targetTextureDesc.sampleCount = 1;
    // At least RenderAttachment usage is needed. Also add CopySrc to be able
    // to retrieve the texture afterwards.
    targetTextureDesc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    targetTextureDesc.viewFormats = NULL;
    targetTextureDesc.viewFormatCount = 0;
    WGPUTexture targetTexture = wgpuDeviceCreateTexture(device, &targetTextureDesc);

    // During initialization
    WGPUTextureViewDescriptor targetTextureViewDesc = {};
    targetTextureViewDesc.nextInChain = NULL;
    targetTextureViewDesc.label = "Render texture view";
    // Render to a single layer
    targetTextureViewDesc.baseArrayLayer = 0;
    targetTextureViewDesc.arrayLayerCount = 1;
    // Render to a single mip level
    targetTextureViewDesc.baseMipLevel = 0;
    targetTextureViewDesc.mipLevelCount = 1;
    // Render to all channels
    targetTextureViewDesc.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetTextureView = wgpuTextureCreateView(targetTexture, &targetTextureViewDesc);

    WGPUTextureDescriptor textureDesc;
    textureDesc.nextInChain = NULL;
    textureDesc.dimension = WGPUTextureDimension_2D;

    WGPUExtent3D patternSize = {};
    patternSize.width = 256;
    patternSize.height = 256;
    patternSize.depthOrArrayLayers = 1;

    textureDesc.size = patternSize;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = NULL;
    WGPUTexture texture = wgpuDeviceCreateTexture(device, &textureDesc);

    // Allocate memory for the pixel array
    uint32_t totalPixels = 4 * textureDesc.size.width * textureDesc.size.height;
    uint8_t *pixels = (uint8_t *)malloc(totalPixels * sizeof(uint8_t));
    if (pixels == NULL) {
        printf("Pixels: Memory allocation failure"); // Handle memory allocation failure
    }

    for (uint32_t i = 0; i < textureDesc.size.width; ++i) {
        for (uint32_t j = 0; j < textureDesc.size.height; ++j) {
            uint8_t *p = &pixels[4 * (j * textureDesc.size.width + i)];
            p[0] = (uint8_t)i; // r
            p[1] = (uint8_t)j; // g
            p[2] = 128; // b
            p[3] = 255; // a
        }
    }

    // Arguments telling which part of the texture we upload to
    // (together with the last argument of writeTexture)
    WGPUImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0;

    WGPUOrigin3D origin = {};
    origin.x = 0;
    origin.y = 0;
    origin.z = 0;
    destination.origin = origin; // equivalent of the offset argument of Queue::writeBuffer
    destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures

    // Arguments telling how the C++ side pixel memory is laid out
    WGPUTextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = 4 * textureDesc.size.width;
    source.rowsPerImage = textureDesc.size.height;

    // wgpuQueueWriteTexture(queue, &destination, &pixels, totalPixels, &source, &(textureDesc.size));

    // In main loop, instead of using swapChain.getCurrentTextureView():
    WGPUTextureView nextTexture = targetTextureView;

    wgpuTextureDestroy(texture);
    wgpuTextureRelease(texture);
}

void triangleDemo(WGPUDevice device, WGPUQueue queue) {
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.nextInChain = NULL;
    
    // [...] Describe render pipeline


    // WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
}

// Run the demo
char *run_demo()
{
    WGPUInstance instance = wgpuCreateInstance(NULL);
    WGPUDevice device = emscripten_webgpu_get_device();
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    commandDemo(device, queue);
    textureDemo(device, queue);
    triangleDemo(device, queue);

    wgpuQueueRelease(queue); // release queue after it's finished

    // return dummy output
    return "Hello from emscripten WebGPU!";
}

static int demo(lua_State *L)
{
    // Call the run_demo function
    char *output = run_demo();

    // lua_pushnumber(L, 0);
    lua_pushstring(L, output);

    return 1;
}


// Library registration function
static const struct luaL_Reg lsokoldemo_funcs[] = {
    {"demo", demo},
    {NULL, NULL} /* Sentinel */
};

// Initialization function
int luaopen_lsokoldemo(lua_State *L)
{
    luaL_newlib(L, lsokoldemo_funcs);
    return 1;
}