#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex;


out vec2 in_tex;

void main()
{
    gl_Position = vec4(position, 1.0);
    in_tex = tex;
}