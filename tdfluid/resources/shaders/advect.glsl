#version 430 core

// implicit inputs
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;

// a workgroup for each pixel
layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// the velocity field
layout (binding = 0, rg32f) uniform image2D uni_velocity;

// the density field
layout (binding = 1, r32f) uniform image2D uni_density;

// the output, advected density field
layout (binding = 2, r32f) uniform image2D uni_buffer;

layout (binding = 3, r32f) uniform image2D uni_mask;

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
    float a = imageLoad(uni_density, ivec2(i, j+1)).r;
    float b = imageLoad(uni_density, ivec2(i+1, j+1)).r;
    float c = imageLoad(uni_density, ivec2(i, j)).r;
    float d = imageLoad(uni_density, ivec2(i+1, j)).r;
    
    float top = (a + (b-a) * delta.x);
    float bottom = (c + (d-c) * delta.x);

    float interpolated = bottom + (top - bottom) * delta.y;
    imageStore(uni_buffer, ij, vec4(interpolated, 0.0, 0.0, 0.0));
}

