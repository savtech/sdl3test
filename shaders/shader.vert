#version 450

layout(set = 1, binding = 0) uniform UBO {
    mat4x4 projection;
};

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_texture_coords;
layout(location = 3) in vec2 in_instance_position;
layout(location = 4) in float in_instance_scaling;

layout (location = 0) out vec4 frag_vertex_color;
layout (location = 1) out vec2 frag_texture_coords;

void main() {
    gl_Position = projection * vec4(in_position + in_instance_position, 0.0, 1.0);
    frag_vertex_color = in_color;
    frag_texture_coords = in_texture_coords;
}