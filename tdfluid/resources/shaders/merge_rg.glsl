#version 430 core

// implicit inputs
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;

// a workgroup for each pixel
layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// the field to copy from
layout (binding = 0, rg32f) uniform image2D uni_from;

// the field to copy to
layout (binding = 1, rg32f) uniform image2D uni_to;

void main()
{
    ivec2 ij = ivec2(gl_GlobalInvocationID.xy);

    vec2 value = imageLoad(uni_from, ij).rg;
    if (length(value) > 0.01)
    {
        imageStore(uni_to, ij, vec4(value, 0.0, 0.0));
    }
}

