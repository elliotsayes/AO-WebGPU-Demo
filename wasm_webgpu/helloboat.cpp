/**
 * This file is part of the "Learn WebGPU for C++" book.
 *   https://github.com/eliemichel/LearnWebGPU
 *
 * MIT License
 * Copyright (c) 2022-2023 Elie Michel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <emscripten.h>
#include "emscripten/html5.h"
#include "emscripten/html5_webgpu.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpuwrapper.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stbimage.h"
#include "stbimagewrite.h"

#include "encodetexture.hpp"

#define RESOURCE_DIR "/data/"

#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <array>

using namespace wgpu;
namespace fs = std::filesystem;
using glm::mat4x4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

constexpr float PI = 3.14159265358979323846f;

/**
 * The same structure as in the shader, replicated in C++
 */
struct MyUniforms
{
	// We add transform matrices
	mat4x4 projectionMatrix;
	mat4x4 viewMatrix;
	mat4x4 modelMatrix;
	std::array<float, 4> color;
	float time;
	float _pad[3];
};

// Have the compiler check byte alignment
static_assert(sizeof(MyUniforms) % 16 == 0);

/**
 * A structure that describes the data layout in the vertex buffer
 * We do not instantiate it but use it in `sizeof` and `offsetof`
 */
struct VertexAttributes
{
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
};

struct PixelData
{
	uint32_t width;
	uint32_t height;
	uint32_t channels;
	unsigned char *data;
};

bool loadShaderDesc(const fs::path &path, ShaderModuleWGSLDescriptor &shaderCodeDesc, ShaderModuleDescriptor &shaderDesc);
bool loadGeometryFromObj(const fs::path &path, std::vector<VertexAttributes> &vertexData);
bool loadPixelData(const fs::path &path, PixelData &pixelData);
Texture createTextureFromPixelData(PixelData &pixelData, Device device, TextureView *pTextureView = nullptr);

// Global flag for whether the data has been loaded
bool dataLoaded = false;

// Globals for ShaderModule, PixelData, and ObjVertexes
char shaderCode[2048];
ShaderModuleWGSLDescriptor shaderCodeDesc;
ShaderModuleDescriptor shaderDesc;
PixelData *pixelData;
std::vector<VertexAttributes> vertexData;

// Global iteration counter
int iteration = 0;

unsigned char *hello_boat(int *len)
{
	iteration++;

	Instance instance = wgpuCreateInstance(NULL);
	Device device = emscripten_webgpu_get_device();
	Queue queue = wgpuDeviceGetQueue(device);

	if (!dataLoaded)
	{
		bool success = false;
		// Load the shader module
		success = loadShaderDesc(RESOURCE_DIR "Ymx4yOfWqgbrmKJDuc2GuN7ZtzwhYs-B10EZlGobiMQ", shaderCodeDesc, shaderDesc);
		if (!success)
		{
			std::cerr << "Could not load shader!" << std::endl;
			return nullptr;
		}

		success = loadPixelData(fs::path(RESOURCE_DIR) / "jBMq9Yuvc98E82j0ChOgtWGIHtEHpA66ZftmN_S2d8U", *pixelData);
		if (!success)
		{
			std::cerr << "Could not load pixel data!" << std::endl;
			return nullptr;
		}

		// Load the geometry
		success = loadGeometryFromObj(RESOURCE_DIR "Di9PS_-XqIqaXbYKHmFST-ftqW5lHcmS0W6BO404I2s", vertexData);
		if (!success)
		{
			std::cerr << "Could not load geometry!" << std::endl;
			return nullptr;
		}
		std::cout << "Loaded " << vertexData.size() << " vertices" << std::endl;

		dataLoaded = true;
	}

	// std::cout << "Creating swapchain..." << std::endl;
#ifdef WEBGPU_BACKEND_WGPU
	// TextureFormat swapChainFormat = surface.getPreferredFormat(adapter);
#else
	TextureFormat swapChainFormat = TextureFormat::RGBA8Unorm;
#endif
	// SwapChainDescriptor swapChainDesc;
	// swapChainDesc.width = 640;
	// swapChainDesc.height = 480;
	// swapChainDesc.usage = TextureUsage::RenderAttachment;
	// swapChainDesc.format = swapChainFormat;
	// swapChainDesc.presentMode = PresentMode::Fifo;
	// SwapChain swapChain = device.createSwapChain(surface, swapChainDesc);
	// std::cout << "Swapchain: " << swapChain << std::endl;

	// During initialization
	TextureDescriptor targetTextureDesc;
	targetTextureDesc.label = "Render target";
	targetTextureDesc.dimension = TextureDimension::_2D;
	// Any size works here, this is the equivalent of the window size
	targetTextureDesc.size = {640, 480, 1};
	// Use the same format here and in the render pipeline's color target
	targetTextureDesc.format = swapChainFormat;
	// No need for MIP maps
	targetTextureDesc.mipLevelCount = 1;
	// You may set up supersampling here
	targetTextureDesc.sampleCount = 1;
	// At least RenderAttachment usage is needed. Also add CopySrc to be able
	// to retrieve the texture afterwards.
	targetTextureDesc.usage = TextureUsage::RenderAttachment | TextureUsage::CopySrc;
	// High fidelity format
	WGPUTextureFormat targetTextureFormat = WGPUTextureFormat_RGBA8Unorm;
	targetTextureDesc.viewFormats = &targetTextureFormat;
	targetTextureDesc.viewFormatCount = 0;
	Texture targetTexture = device.createTexture(targetTextureDesc);

	// During initialization
	TextureViewDescriptor targetTextureViewDesc;
	targetTextureViewDesc.label = "Render texture view";
	// Render to a single layer
	targetTextureViewDesc.baseArrayLayer = 0;
	targetTextureViewDesc.arrayLayerCount = 1;
	// Render to a single mip level
	targetTextureViewDesc.baseMipLevel = 0;
	targetTextureViewDesc.mipLevelCount = 1;
	// Render to all channels
	targetTextureViewDesc.aspect = TextureAspect::All;
	TextureView targetTextureView = targetTexture.createView(targetTextureViewDesc);

	std::cout << "Creating shader module..." << std::endl;
	ShaderModule shaderModule = device.createShaderModule(shaderDesc);
	emscripten_sleep(0);
	std::cout << "Shader module: " << shaderModule << std::endl;

	std::cout << "Creating render pipeline..." << std::endl;
	RenderPipelineDescriptor pipelineDesc;

	// Vertex fetch
	std::vector<VertexAttribute> vertexAttribs(4);

	// Position attribute
	vertexAttribs[0].shaderLocation = 0;
	vertexAttribs[0].format = VertexFormat::Float32x3;
	vertexAttribs[0].offset = 0;

	// Normal attribute
	vertexAttribs[1].shaderLocation = 1;
	vertexAttribs[1].format = VertexFormat::Float32x3;
	vertexAttribs[1].offset = offsetof(VertexAttributes, normal);

	// Color attribute
	vertexAttribs[2].shaderLocation = 2;
	vertexAttribs[2].format = VertexFormat::Float32x3;
	vertexAttribs[2].offset = offsetof(VertexAttributes, color);

	// UV attribute
	vertexAttribs[3].shaderLocation = 3;
	vertexAttribs[3].format = VertexFormat::Float32x2;
	vertexAttribs[3].offset = offsetof(VertexAttributes, uv);

	VertexBufferLayout vertexBufferLayout;
	vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
	vertexBufferLayout.attributes = vertexAttribs.data();
	vertexBufferLayout.arrayStride = sizeof(VertexAttributes);
	vertexBufferLayout.stepMode = VertexStepMode::Vertex;

	pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;

	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = FrontFace::CCW;
	pipelineDesc.primitive.cullMode = CullMode::None;

	FragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	BlendState blendState;
	blendState.color.srcFactor = BlendFactor::SrcAlpha;
	blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = BlendOperation::Add;
	blendState.alpha.srcFactor = BlendFactor::Zero;
	blendState.alpha.dstFactor = BlendFactor::One;
	blendState.alpha.operation = BlendOperation::Add;

	ColorTargetState colorTarget;
	colorTarget.format = swapChainFormat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = ColorWriteMask::All;

	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	DepthStencilState depthStencilState = Default;
	depthStencilState.depthCompare = CompareFunction::Less;
	depthStencilState.depthWriteEnabled = true;
	// High fidelity format
	TextureFormat depthTextureFormat = TextureFormat::Depth32Float;
	depthStencilState.format = depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

	pipelineDesc.depthStencil = &depthStencilState;

	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// Create binding layouts

	// Since we now have 2 bindings, we use a vector to store them
	std::vector<BindGroupLayoutEntry> bindingLayoutEntries(3, Default);

	// The uniform buffer binding that we already had
	BindGroupLayoutEntry &bindingLayout = bindingLayoutEntries[0];
	bindingLayout.binding = 0;
	bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
	bindingLayout.buffer.type = BufferBindingType::Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(MyUniforms);

	// The texture binding
	BindGroupLayoutEntry &textureBindingLayout = bindingLayoutEntries[1];
	textureBindingLayout.binding = 1;
	textureBindingLayout.visibility = ShaderStage::Fragment;
	textureBindingLayout.texture.sampleType = TextureSampleType::Float;
	textureBindingLayout.texture.viewDimension = TextureViewDimension::_2D;

	// The texture sampler binding
	BindGroupLayoutEntry &samplerBindingLayout = bindingLayoutEntries[2];
	samplerBindingLayout.binding = 2;
	samplerBindingLayout.visibility = ShaderStage::Fragment;
	samplerBindingLayout.sampler.type = SamplerBindingType::Filtering;

	// Create a bind group layout
	BindGroupLayoutDescriptor bindGroupLayoutDesc{};
	bindGroupLayoutDesc.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDesc.entries = bindingLayoutEntries.data();
	BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

	// Create the pipeline layout
	PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *)&bindGroupLayout;
	PipelineLayout layout = device.createPipelineLayout(layoutDesc);
	pipelineDesc.layout = layout;

	RenderPipeline pipeline = device.createRenderPipeline(pipelineDesc);
	std::cout << "Render pipeline: " << pipeline << std::endl;

	// Create the depth texture
	TextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = TextureDimension::_2D;
	depthTextureDesc.format = depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = {640, 480, 1};
	depthTextureDesc.usage = TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat *)&depthTextureFormat;
	Texture depthTexture = device.createTexture(depthTextureDesc);
	std::cout << "Depth texture: " << depthTexture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	TextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = TextureViewDimension::_2D;
	depthTextureViewDesc.format = depthTextureFormat;
	TextureView depthTextureView = depthTexture.createView(depthTextureViewDesc);
	std::cout << "Depth texture view: " << depthTextureView << std::endl;

	// Create a sampler
	SamplerDescriptor samplerDesc;
	samplerDesc.addressModeU = AddressMode::Repeat;
	samplerDesc.addressModeV = AddressMode::Repeat;
	samplerDesc.addressModeW = AddressMode::Repeat;
	samplerDesc.magFilter = FilterMode::Linear;
	samplerDesc.minFilter = FilterMode::Linear;
	samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
	samplerDesc.lodMinClamp = 0.0f;
	samplerDesc.lodMaxClamp = 8.0f;
	samplerDesc.compare = CompareFunction::Undefined;
	samplerDesc.maxAnisotropy = 1;
	Sampler sampler = device.createSampler(samplerDesc);

	// Create a texture
	TextureView textureView = nullptr;
	// Texture texture = loadPixelData(RESOURCE_DIR "jBMq9Yuvc98E82j0ChOgtWGIHtEHpA66ZftmN_S2d8U", device, &textureView);
	Texture texture = createTextureFromPixelData(*pixelData, device, &textureView);
	if (!texture)
	{
		std::cerr << "Could not load texture!" << std::endl;
		return nullptr;
	}
	std::cout << "Texture: " << texture << std::endl;
	std::cout << "Texture view: " << textureView << std::endl;

	// Create vertex buffer
	BufferDescriptor bufferDesc;
	bufferDesc.size = vertexData.size() * sizeof(VertexAttributes);
	bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
	bufferDesc.mappedAtCreation = false;
	Buffer vertexBuffer = device.createBuffer(bufferDesc);
	queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);

	int indexCount = static_cast<int>(vertexData.size());

	// Create uniform buffer
	bufferDesc.size = sizeof(MyUniforms);
	bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
	bufferDesc.mappedAtCreation = false;
	Buffer uniformBuffer = device.createBuffer(bufferDesc);

	// Rotate around in a circle
	float x = 3.0f * cos(iteration * 0.1f);
	float y = 3.0f * sin(iteration * 0.1f);

	// Upload the initial value of the uniforms
	MyUniforms uniforms;
	uniforms.modelMatrix = mat4x4(1.0);
	uniforms.viewMatrix = glm::lookAt(vec3(-x, -y, 2.0f), vec3(0.0f), vec3(0, 0, 1));
	uniforms.projectionMatrix = glm::perspective(45 * PI / 180, 640.0f / 480.0f, 0.01f, 100.0f);
	uniforms.time = 1.0f;
	uniforms.color = {0.0f, 1.0f, 0.4f, 1.0f};
	std::cout << "Uniform buffer: " << uniformBuffer << std::endl;
	queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(MyUniforms));
	std::cout << "Uniforms: " << uniforms.time << std::endl;

	// Create a binding
	std::vector<BindGroupEntry> bindings(3);

	bindings[0].binding = 0;
	bindings[0].buffer = uniformBuffer;
	bindings[0].offset = 0;
	bindings[0].size = sizeof(MyUniforms);

	bindings[1].binding = 1;
	bindings[1].textureView = textureView;

	bindings[2].binding = 2;
	bindings[2].sampler = sampler;

	BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = (uint32_t)bindings.size();
	bindGroupDesc.entries = bindings.data();
	BindGroup bindGroup = device.createBindGroup(bindGroupDesc);

	{
		// glfwPollEvents();

		// Update uniform buffer
		uniforms.time = 10; // static_cast<float>(glfwGetTime());
		queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, time), &uniforms.time, sizeof(MyUniforms::time));

		// TextureView nextTexture = swapChain.getCurrentTextureView();
		// if (!nextTexture) {
		// 	std::cerr << "Cannot acquire next swap chain texture" << std::endl;
		// 	return nullptr;
		// }

		// In main loop, instead of using swapChain.getCurrentTextureView():
		TextureView nextTexture = targetTextureView;

		CommandEncoderDescriptor commandEncoderDesc;
		commandEncoderDesc.label = "Command Encoder";
		CommandEncoder encoder = device.createCommandEncoder(commandEncoderDesc);

		RenderPassDescriptor renderPassDesc{};

		RenderPassColorAttachment renderPassColorAttachment{};
		renderPassColorAttachment.view = nextTexture;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = LoadOp::Clear;
		renderPassColorAttachment.storeOp = StoreOp::Store;
		renderPassColorAttachment.clearValue = Color{0.05, 0.05, 0.05, 1.0};
		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

		RenderPassDepthStencilAttachment depthStencilAttachment;
		depthStencilAttachment.view = depthTextureView;
		depthStencilAttachment.depthClearValue = 1.0f;
		depthStencilAttachment.depthLoadOp = LoadOp::Clear;
		depthStencilAttachment.depthStoreOp = StoreOp::Store;
		depthStencilAttachment.depthReadOnly = false;
		depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
		depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
		depthStencilAttachment.stencilStoreOp = StoreOp::Store;
#else
		depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
		depthStencilAttachment.stencilStoreOp = StoreOp::Undefined;
#endif
		depthStencilAttachment.stencilReadOnly = true;

		renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

		// renderPassDesc.timestampWriteCount = 0;
		renderPassDesc.timestampWrites = nullptr;
		RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

		renderPass.setPipeline(pipeline);

		renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexData.size() * sizeof(VertexAttributes));

		// Set binding group
		renderPass.setBindGroup(0, bindGroup, 0, nullptr);

		renderPass.draw(indexCount, 1, 0, 0);

		renderPass.end();
		renderPass.release();

		// nextTexture.release();

		CommandBufferDescriptor cmdBufferDescriptor{};
		cmdBufferDescriptor.label = "Command buffer";
		CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();
		queue.submit(command);
		command.release();

		// swapChain.present();

		// #ifdef WEBGPU_BACKEND_DAWN
		// 		// Check for pending error callbacks
		// 		device.tick();
		// #endif
		emscripten_sleep(0);
	}

	vertexBuffer.destroy();
	vertexBuffer.release();

	texture.destroy();
	texture.release();

	// Destroy the depth texture and its view
	depthTextureView.release();
	depthTexture.destroy();
	depthTexture.release();

	pipeline.release();
	shaderModule.release();
	// swapChain.release();
	// device.release();
	// adapter.release();
	// instance.release();
	// surface.release();

	emscripten_sleep(0);

	// Instead of swapChain.present()
	unsigned char *png = encodeTexturePng(device, targetTexture, len);

	// // Dummy value "myfakepng"
	// unsigned char *png = (unsigned char *)"myfakepng";
	// *len = 10;

	// saveTextureView("output.png", device, nextTexture, targetTexture.getWidth(), targetTexture.getHeight());

	// pipeline.release();
	// shaderModule.release();
	// device.release();
	// instance.release();
	// surface.release();

	return png;
}

bool loadShaderDesc(const fs::path &path, ShaderModuleWGSLDescriptor &shaderCodeDesc, ShaderModuleDescriptor &shaderDesc)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return false;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::cout << "Shader size: " << size << std::endl;
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);
	strncpy(shaderCode, shaderSource.c_str(), sizeof(shaderCode) - 1);
	shaderCode[sizeof(shaderCode) - 1] = '\0'; // Ensure null-termination

	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
	shaderCodeDesc.code = shaderCode;

	shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	return true;
}

bool loadGeometryFromObj(const fs::path &path, std::vector<VertexAttributes> &vertexData)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::cout << "Loading OBJ file: " << path << std::endl;
	// Call the core loading procedure of TinyOBJLoader
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), RESOURCE_DIR);
	std::cout << "Loaded OBJ" << std::endl;

	// Check errors
	if (!warn.empty())
	{
		std::cout << warn << std::endl;
	}

	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		return false;
	}

	// Filling in vertexData:
	vertexData.clear();
	for (const auto &shape : shapes)
	{
		size_t offset = vertexData.size();
		vertexData.resize(offset + shape.mesh.indices.size());

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i)
		{
			const tinyobj::index_t &idx = shape.mesh.indices[i];

			vertexData[offset + i].position = {
					attrib.vertices[3 * idx.vertex_index + 0],
					-attrib.vertices[3 * idx.vertex_index + 2],
					attrib.vertices[3 * idx.vertex_index + 1]};

			vertexData[offset + i].normal = {
					attrib.normals[3 * idx.normal_index + 0],
					-attrib.normals[3 * idx.normal_index + 2],
					attrib.normals[3 * idx.normal_index + 1]};

			vertexData[offset + i].color = {
					attrib.colors[3 * idx.vertex_index + 0],
					attrib.colors[3 * idx.vertex_index + 1],
					attrib.colors[3 * idx.vertex_index + 2]};

			vertexData[offset + i].uv = {
					attrib.texcoords[2 * idx.texcoord_index + 0],
					1 - attrib.texcoords[2 * idx.texcoord_index + 1]};
		}
	}

	return true;
}

// Auxiliary function for mip map generation
static void writeMipMaps(
		Device device,
		Texture texture,
		Extent3D textureSize,
		uint32_t mipLevelCount,
		const unsigned char *pixelData)
{
	Queue queue = device.getQueue();

	// Arguments telling which part of the texture we upload to
	ImageCopyTexture destination;
	destination.texture = texture;
	destination.origin = {0, 0, 0};
	destination.aspect = TextureAspect::All;

	// Arguments telling how the C++ side pixel memory is laid out
	TextureDataLayout source;
	source.offset = 0;

	// Create image data
	Extent3D mipLevelSize = textureSize;
	std::vector<unsigned char> previousLevelPixels;
	Extent3D previousMipLevelSize;
	for (uint32_t level = 0; level < mipLevelCount; ++level)
	{
		// Pixel data for the current level
		std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
		if (level == 0)
		{
			// We cannot really avoid this copy since we need this
			// in previousLevelPixels at the next iteration
			memcpy(pixels.data(), pixelData, pixels.size());
		}
		else
		{
			// Create mip level data
			for (uint32_t i = 0; i < mipLevelSize.width; ++i)
			{
				for (uint32_t j = 0; j < mipLevelSize.height; ++j)
				{
					unsigned char *p = &pixels[4 * (j * mipLevelSize.width + i)];
					// Get the corresponding 4 pixels from the previous level
					unsigned char *p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char *p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
					unsigned char *p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char *p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
					// Average
					p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
					p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
					p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
					p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
				}
			}
		}

		// Upload data to the GPU texture
		destination.mipLevel = level;
		source.bytesPerRow = 4 * mipLevelSize.width;
		source.rowsPerImage = mipLevelSize.height;
		queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

		previousLevelPixels = std::move(pixels);
		previousMipLevelSize = mipLevelSize;
		mipLevelSize.width /= 2;
		mipLevelSize.height /= 2;
	}

	queue.release();
}

// Equivalent of std::bit_width that is available from C++20 onward
static uint32_t bit_width(uint32_t m)
{
	if (m == 0)
		return 0;
	else
	{
		uint32_t w = 0;
		while (m >>= 1)
			++w;
		return w;
	}
}

bool loadPixelData(const fs::path &path, PixelData &pixelData)
{
	int width, height, channels;
	// Load from memory instead
	// unsigned char *pixelData = stbi_load(path.string().c_str(), &width, &height, &channels, 4 /* force 4 channels */);
	std::ifstream file(path);
	if (!file.is_open())
	{
		return false;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::cout << "Image size: " << size << std::endl;
	// Binary data
	unsigned char *imageData = new unsigned char[size];
	file.seekg(0);
	file.read((char *)imageData, size);
	file.close();

	unsigned char *data = stbi_load_from_memory(imageData, size, &width, &height, &channels, 4 /* force 4 channels */);

	if (!data)
	{
		std::cerr << "Could not load image!" << std::endl;
		return false;
	}

	pixelData.width = width;
	pixelData.height = height;
	pixelData.channels = channels;
	pixelData.data = data;

	return true;
}

Texture createTextureFromPixelData(PixelData &pixelData, Device device, TextureView *pTextureView)
{
	// Use the width, height, channels and data variables here
	TextureDescriptor textureDesc;
	textureDesc.dimension = TextureDimension::_2D;
	textureDesc.format = TextureFormat::RGBA8Unorm; // by convention for bmp, png and jpg file. Be careful with other formats.
	textureDesc.size = {(unsigned int)(pixelData.width), (unsigned int)(pixelData.height), 1};
	textureDesc.mipLevelCount = bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
	textureDesc.sampleCount = 1;
	textureDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
	textureDesc.viewFormatCount = 0;
	// High fidelity format
	textureDesc.viewFormats = &textureDesc.format;
	Texture texture = device.createTexture(textureDesc);

	// Upload data to the GPU texture
	writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, pixelData.data);

	if (pTextureView)
	{
		TextureViewDescriptor textureViewDesc;
		textureViewDesc.aspect = TextureAspect::All;
		textureViewDesc.baseArrayLayer = 0;
		textureViewDesc.arrayLayerCount = 1;
		textureViewDesc.baseMipLevel = 0;
		textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
		textureViewDesc.dimension = TextureViewDimension::_2D;
		textureViewDesc.format = textureDesc.format;
		*pTextureView = texture.createView(textureViewDesc);
	}

	return texture;
}

// extern c function
extern "C" unsigned char *run_hello_boat(int *len)
{
	return hello_boat(len);
}
