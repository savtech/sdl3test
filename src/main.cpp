#include "shaders.h"
#include "textures.h"

#include <orshlib.h>
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdio>

using namespace OL;

struct Resolution {
    u32 width;
    u32 height;
};

static constexpr Resolution resolutions[] = {
    { 640, 360 },
    { 1024, 576 },
    { 1366, 768 },
    { 1920, 1080 }
};

static Resolution resolution = resolutions[0];

struct Vertex {
    Vector2 position;
    Color color;
    Vector2 texture_coords;
};

static constexpr u16 indices[] = {
    0, 1, 2, 0, 2, 3
};

static constexpr size_t MAX_ENTITIES = 500000;

struct InstanceData {
    Vector2 position;
    //f32 scaling;
};

struct Entity {
    Vector2 velocity;
};

static InstanceData instances[MAX_ENTITIES];
static Entity entities[MAX_ENTITIES];

static SDL_GPUDevice* device = nullptr;

struct GPUBuffer {
    SDL_GPUBuffer* buffer;
    SDL_GPUBufferUsageFlags usage;
};

template<typename T>
void upload_gpu_data(T* data, u32 size, GPUBuffer* buffer_info) {
    SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = size
    };

    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_buffer_info);
    void* transfer_destination = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    memcpy_s(transfer_destination, size, data, size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    SDL_GPUTransferBufferLocation transfer_buffer_location = {
        .transfer_buffer = transfer_buffer
    };

    if(buffer_info->buffer == nullptr) {
        SDL_GPUBufferCreateInfo buffer_create_info = {
            .usage = buffer_info->usage,
            .size = size
        };

        buffer_info->buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);
    }

    SDL_GPUBufferRegion buffer_region = {
        .buffer = buffer_info->buffer,
        .size = size
    };

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    SDL_UploadToGPUBuffer(copy_pass, &transfer_buffer_location, &buffer_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
}

Vertex* calculate_texture_vertices(TextureAtlas texture_atlas) {
    static constexpr Vertex normalized_quad_vertices[] = {
        { { -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }, // Bottom-left
        { { 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },  // Bottom-right
        { { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },   // Top-right
        { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }   // Top-left
    };

    size_t quad_vertices_size = sizeof(normalized_quad_vertices);

    Vertex* buffer = memory.persistent->reserve<Vertex>(texture_atlas.count * 4);
    for(u32 texture_index = 0; texture_index < texture_atlas.count; ++texture_index) {
        Vertex* quad_vertices = buffer + (texture_index * 4);
        memcpy_s(quad_vertices, quad_vertices_size, normalized_quad_vertices, quad_vertices_size);
        Texture* texture = &texture_atlas.textures[texture_index];
        f32 texture_aspect_ratio = static_cast<f32>(texture->width) / texture->height;
        f32 resolution_aspect_ratio = static_cast<f32>(resolution.width) / resolution.height;

        for(u32 i = 0; i < 4; ++i) {
            quad_vertices[i].position.x *= texture_aspect_ratio / resolution_aspect_ratio;
            quad_vertices[i].position.x *= 0.4f;
            quad_vertices[i].position.y *= 0.4f;
        }
    }

    return buffer;
}

void init_entities() {
    for(size_t i = 0; i < MAX_ENTITIES; ++i) {
        Entity* entity = &entities[i];
        entity->velocity = { Random::F32(-0.00015f, 0.00015f), Random::F32(-0.00015f, 0.00015f) };
        instances[i] = {
            .position = { Random::F32(-1.0f, 1.0f), Random::F32(-1.0f, 1.0f) },
            //.scaling = Random::F32(0.1f, 0.5f)
        };
    }
}

int main() {
    SDL_SetAppMetadata("SDL3 Test", "1.0", "com.savtech.test");

    SDL_Init(SDL_INIT_EVENTS);

    SDL_Window* window = SDL_CreateWindow("AzenrisPls", resolution.width, resolution.height, SDL_WINDOW_VULKAN);
    assert(window);

    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, "vulkan");
    assert(device);

    if(!SDL_ClaimWindowForGPUDevice(device, window)) {
        printf("oh no device cant fuck the window\n");
    }

    SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

    TextureAtlas texture_atlas = load_textures(device);
    Vertex* texture_vertices = calculate_texture_vertices(texture_atlas);

    GPUBuffer vertex_buffer = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX
    };
    upload_gpu_data(texture_vertices, sizeof(Vertex) * texture_atlas.count * 4, &vertex_buffer);

    init_entities();

    GPUBuffer instance_buffer = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX
    };
    upload_gpu_data(instances, sizeof(instances), &instance_buffer);

    SDL_GPUBufferBinding vertex_buffer_bindings[] = {
        { .buffer = vertex_buffer.buffer },
        { .buffer = instance_buffer.buffer }
    };

    GPUBuffer index_buffer = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX
    };
    upload_gpu_data(indices, sizeof(indices), &index_buffer);

    SDL_GPUBufferBinding index_binding = {
        .buffer = index_buffer.buffer
    };

    load_shaders(device);

    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[] = {
        { .slot = 0,
          .pitch = sizeof(Vertex),
          .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX },
        { .slot = 1,
          .pitch = sizeof(Vector2),
          .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
          .instance_step_rate = 1 }
    };

    SDL_GPUVertexAttribute vertex_attributes[] = {
        { .location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
        { .location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(Vertex, color) },
        { .location = 2, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = offsetof(Vertex, texture_coords) },
        { .location = 3, .buffer_slot = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
        //{ .location = 4, .buffer_slot = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, .offset = offsetof(InstanceData, scaling) },
    };

    SDL_GPUVertexInputState vertex_input_state = {
        .vertex_buffer_descriptions = vertex_buffer_descriptions,
        .num_vertex_buffers = static_cast<u32>(OL::array_count(vertex_buffer_descriptions)),
        .vertex_attributes = vertex_attributes,
        .num_vertex_attributes = static_cast<u32>(OL::array_count(vertex_attributes))
    };

    SDL_GPURasterizerState rasterizer_state = {
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_BACK,
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
    };

    SDL_GPUColorTargetDescription color_target_desc = {
        .format = SDL_GetGPUSwapchainTextureFormat(device, window)
    };

    SDL_GPUGraphicsPipelineCreateInfo graphics_pipeline_info = {
        .vertex_shader = shaders[0],
        .fragment_shader = shaders[1],
        .vertex_input_state = vertex_input_state,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = rasterizer_state,
        .target_info = {
            .color_target_descriptions = &color_target_desc,
            .num_color_targets = 1 }
    };
    SDL_GPUGraphicsPipeline* graphics_pipeline = SDL_CreateGPUGraphicsPipeline(device, &graphics_pipeline_info);

    release_shaders(device);

    SDL_GPUSamplerCreateInfo texture_sampler_info = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
    };

    SDL_GPUSampler* texture_sampler = SDL_CreateGPUSampler(device, &texture_sampler_info);
    assert(texture_sampler);

    SDL_GPUTextureSamplerBinding texture_sampler_binding = {
        .texture = texture_atlas.textures[2].handle,
        .sampler = texture_sampler
    };

    SDL_GPUIndexedIndirectDrawCommand draw_command {
        .num_indices = 6,
        .num_instances = static_cast<u32>(OL::array_count(instances)),
        .first_index = 0,
        .vertex_offset = 8,
        .first_instance = 0
    };

    GPUBuffer draw_buffer = {
        .usage = SDL_GPU_BUFFERUSAGE_INDIRECT
    };
    upload_gpu_data(&draw_command, sizeof(draw_command), &draw_buffer);

    bool running = true;
    bool render = true;
    SDL_Event event;

    Session session = {};
    Time::Stamp current_time = Time::Clock::now();
    Time::Duration accumulator = Time::Duration::zero();
    Time::Duration delta_time = Time::Milliseconds(10);

    while(running) {
        Time::Stamp new_time = Time::Clock::now();
        Time::Duration frame_time = new_time - current_time;
        current_time = new_time;
        accumulator += frame_time;

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EventType::SDL_EVENT_QUIT: {
                    running = false;
                } break;
                case SDL_EventType::SDL_EVENT_WINDOW_MINIMIZED: {
                    render = false;
                } break;
                case SDL_EventType::SDL_EVENT_WINDOW_RESTORED: {
                    render = true;
                } break;
                case SDL_EventType::SDL_EVENT_KEY_UP:
                case SDL_EventType::SDL_EVENT_KEY_DOWN: {
                    SDL_KeyboardEvent* keyboard_event = reinterpret_cast<SDL_KeyboardEvent*>(&event);
                    switch(keyboard_event->key) {
                        case SDLK_ESCAPE: {
                            running = false;
                        } break;
                        default:
                            break;
                    }
                } break;
                default:
                    continue;
            }
        }

        while(accumulator >= delta_time) {
            session.update(delta_time);

            for(size_t i = 0; i < MAX_ENTITIES; ++i) {
                instances[i].position += entities[i].velocity;
            }

            accumulator -= delta_time;
        }

        upload_gpu_data(instances, sizeof(instances), &instance_buffer);

        //Render
        if(render) {
            SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);

            SDL_GPUTexture* render_texture;
            u32 render_width, render_height;
            if(!SDL_AcquireGPUSwapchainTexture(command_buffer, window, &render_texture, &render_width, &render_height)) {
                Logger::log("uh oh swapchain is fucked");
            }
            SDL_GPUColorTargetInfo render_target_info = {
                .texture = render_texture,
                .clear_color = { 0.2f, 0.2f, 0.2f, 1.0f },
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE
            };

            SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &render_target_info, 1, nullptr);
            SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);
            SDL_BindGPUVertexBuffers(render_pass, 0, vertex_buffer_bindings, static_cast<u32>(OL::array_count(vertex_buffer_bindings)));
            SDL_BindGPUIndexBuffer(render_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
            SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_sampler_binding, 1);

            //Matrix4 projection = orthographic_projection(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
            SDL_PushGPUVertexUniformData(command_buffer, 0, &ORTHO, sizeof(Matrix4));

            //SDL_DrawGPUIndexedPrimitives(render_pass, 6, static_cast<u32>(OL::array_count(instances)), 0, 8, 0);
            SDL_DrawGPUIndexedPrimitivesIndirect(render_pass, draw_buffer.buffer, 0, 1);

            SDL_EndGPURenderPass(render_pass);
            SDL_SubmitGPUCommandBuffer(command_buffer);

            session.render(delta_time);
            static char fps[32];
            snprintf(fps, 32, "FPS: %f", session.fps.last_measurement);
            SDL_SetWindowTitle(window, fps);
        }
    }

    session.debug_print();

    SDL_Quit();
    return 0;
}