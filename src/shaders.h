#pragma once

#include <orshlib/file.h>
#include <SDL3/SDL_gpu.h>

static const char* SHADER_DIRECTORY = "shaders/compiled/";

struct ShaderData {
    const char* name;
    SDL_GPUShaderStage stage;
    u32 samplers_count;
    u32 storage_textures_count;
    u32 storage_buffers_count;
    u32 uniform_buffers_count;
};

static ShaderData shader_data[] = {
    { .name = "shader_vert",
      .stage = SDL_GPU_SHADERSTAGE_VERTEX,
      .uniform_buffers_count = 1 },
    { .name = "shader_frag",
      .stage = SDL_GPU_SHADERSTAGE_VERTEX,
      .samplers_count = 1 }
};

static SDL_GPUShader* shaders[OL::array_count(shader_data)];

static SDL_GPUShader* load_shader(SDL_GPUDevice* device, ShaderData data) {
    char path[OL::File::MAX_FILENAME_LENGTH];
    snprintf(path, OL::File::MAX_FILENAME_LENGTH, "%s%s%s", SHADER_DIRECTORY, data.name, ".spv");

    OL::File* file = OL::File::load(path);
    assert(file);

    SDL_GPUShaderCreateInfo shader_info = {
        .code_size = file->size(),
        .code = file->data(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = data.stage,
        .num_samplers = data.samplers_count,
        .num_storage_textures = data.storage_textures_count,
        .num_storage_buffers = data.storage_buffers_count,
        .num_uniform_buffers = data.uniform_buffers_count
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shader_info);
    assert(shader);

    return shader;
};

static void load_shaders(SDL_GPUDevice* device) {
    for(u32 i = 0; i < OL::array_count(shader_data); ++i) {
        shaders[i] = load_shader(device, shader_data[i]);
    }
};

static void release_shaders(SDL_GPUDevice* device) {
    for(u32 i = 0; i < OL::array_count(shaders); ++i) {
        SDL_ReleaseGPUShader(device, shaders[i]);
    }
};