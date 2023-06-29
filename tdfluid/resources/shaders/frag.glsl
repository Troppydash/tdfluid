#version 430 core

out vec4 out_color;

in vec2 in_tex;
layout (binding = 0) uniform sampler2D uni_texture;

void main()
{
    out_color = texture(uni_texture, in_tex);
}