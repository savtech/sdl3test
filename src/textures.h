#pragma once

#include <orshlib/file.h>
#include <SDL3/SDL_gpu.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
//#define STBI_ONLY_PNG
#include <stb_image.h>

static const char* ASSETS_DIRECTORY = "assets/";

static const char* image_paths[] = {
    "athano.bmp",
    "azen.png",
    "azen2.png",
    "azen3.png",
    "pepe.png"
};

static constexpr size_t MAX_TEXTURES_COUNT = OL::array_count(image_paths);

struct ImageData {
    u8* pixels;
    u32 size;
    u32 width;
    u32 height;
};

struct Texture {
    size_t id;
    SDL_GPUTexture* handle;
    u32 width;
    u32 height;
};

struct TextureAtlas {
    Texture* textures;
    u32 count;
};

static TextureAtlas load_textures(SDL_GPUDevice* device) {
    TextureAtlas atlas = {
        .textures = OL::memory.persistent->reserve<Texture>(MAX_TEXTURES_COUNT),
        .count = MAX_TEXTURES_COUNT
    };
    assert(atlas.textures);

    ImageData images_data[MAX_TEXTURES_COUNT];
    u32 images_size = 0;
    for(size_t image_index = 0; image_index < MAX_TEXTURES_COUNT; ++image_index) {
        ImageData* data = &images_data[image_index];

        char path[OL::File::MAX_FILENAME_LENGTH];
        snprintf(path, OL::File::MAX_FILENAME_LENGTH, "%s%s", ASSETS_DIRECTORY, image_paths[image_index]);

        s32 width, height;
        data->pixels = stbi_load(path, &width, &height, nullptr, 4);

        data->width = static_cast<u32>(width);
        data->height = static_cast<u32>(height);
        data->size = data->width * data->height * 4;

        images_size += data->size;
    }

    SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = images_size
    };

    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_buffer_info);
    void* transfer_data = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    u32 buffer_offset = 0;
    for(size_t image_index = 0; image_index < MAX_TEXTURES_COUNT; ++image_index) {
        ImageData* data = &images_data[image_index];
        u8* slice = reinterpret_cast<u8*>(transfer_data) + buffer_offset;
        memcpy_s(slice, images_size, data->pixels, data->size);
        buffer_offset += data->size;
    }
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    buffer_offset = 0;
    SDL_GPUTexture* gpu_textures[MAX_TEXTURES_COUNT];
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    for(size_t image_index = 0; image_index < MAX_TEXTURES_COUNT; ++image_index) {
        ImageData* data = &images_data[image_index];
        SDL_GPUTextureTransferInfo texture_transfer_info = {
            .transfer_buffer = transfer_buffer,
            .offset = buffer_offset,
            .pixels_per_row = data->width
        };

        buffer_offset += data->size;

        SDL_GPUTextureCreateInfo texture_info = {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = data->width,
            .height = data->height,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .sample_count = SDL_GPU_SAMPLECOUNT_1
        };

        SDL_GPUTexture* gpu_texture = SDL_CreateGPUTexture(device, &texture_info);
        assert(gpu_texture);

        SDL_GPUTextureRegion texture_region = {
            .texture = gpu_texture,
            .w = data->width,
            .h = data->height,
            .d = 1
        };

        gpu_textures[image_index] = gpu_texture;
        SDL_UploadToGPUTexture(copy_pass, &texture_transfer_info, &texture_region, false);
    }

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);

    for(size_t image_index = 0; image_index < MAX_TEXTURES_COUNT; ++image_index) {
        ImageData* data = &images_data[image_index];
        Texture* texture = &atlas.textures[image_index];

        texture->id = image_index;
        texture->handle = gpu_textures[image_index];
        texture->width = data->width;
        texture->height = data->height;

        stbi_image_free(data->pixels);
    }

    return atlas;
}