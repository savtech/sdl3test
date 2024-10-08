#version 450

layout(set = 2, binding = 0) uniform sampler2D texture_sampler;

layout(location = 0) in vec4 frag_vertex_color;
layout(location = 1) in vec2 frag_texture_coords;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(texture_sampler, frag_texture_coords);
    if(out_color.w < 1.0) {
        discard;
    }
}