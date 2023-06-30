#version 430 core

// implicit inputs
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;

// a workgroup for each pixel
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// the input pressure field
layout (binding = 0, r32f) uniform image2D uni_pressure;

// the input/output velocity field to project
layout (binding = 1, rg32f) uniform image2D uni_velocity;

layout (binding = 2, r32f) uniform image2D uni_mask;

void main()
{
    ivec2 ij = ivec2(gl_GlobalInvocationID.xy);

    float mask = imageLoad(uni_mask, ij).r;
    if (mask > 0.5)
    {
        return;
    }

    int i = ij.x;
    int j = ij.y;

    // compute pressure gradient
    float dx = imageLoad(uni_pressure, ivec2(i+1,j)).r - imageLoad(uni_pressure, ivec2(i-1,j)).r;
    float dy = imageLoad(uni_pressure, ivec2(i,j+1)).r - imageLoad(uni_pressure, ivec2(i,j-1)).r;

    // subtract pressure gradient from velocity field
    vec2 new_velocity = imageLoad(uni_velocity, ij).rg - vec2(dx, dy);
    imageStore(uni_velocity, ij, vec4(new_velocity, 0.0, 0.0));
}

