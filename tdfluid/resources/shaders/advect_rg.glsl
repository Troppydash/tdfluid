#version 430 core

// implicit inputs
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;

// a workgroup for each pixel
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// the velocity field
layout (binding = 0, rg32f) uniform image2D uni_velocity;

// the output, advected density field
layout (binding = 1, rg32f) uniform image2D uni_buffer;

layout (binding = 2, r32f) uniform image2D uni_mask;

// delta time
uniform float dt;

void main()
{
    ivec2 ij = ivec2(gl_GlobalInvocationID.xy);

    float mask = imageLoad(uni_mask, ij).r;
    if (mask > 0.5)
    {
        return;
    }

    // backwards euler step to find the source position
    vec2 velocity = imageLoad(uni_velocity, ij).rg;
    vec2 position = vec2(ij) - dt * velocity;

    // linearly interpolate the nearby density values
    vec2 anchor = floor(position);
    vec2 delta = position - anchor;

    int i = int(anchor.x);
    int j = int(anchor.y);

    // [a, b]
    // [c, d]
    vec2 a = imageLoad(uni_velocity, ivec2(i, j+1)).rg;
    vec2 b = imageLoad(uni_velocity, ivec2(i+1, j+1)).rg;
    vec2 c = imageLoad(uni_velocity, ivec2(i, j)).rg;
    vec2 d = imageLoad(uni_velocity, ivec2(i+1, j)).rg;
    
    vec2 top = (a + (b-a) * delta.x);
    vec2 bottom = (c + (d-c) * delta.x);

    vec2 interpolated = bottom + (top - bottom) * delta.y;
    imageStore(uni_buffer, ij, vec4(interpolated, 0.0, 0.0));
}

