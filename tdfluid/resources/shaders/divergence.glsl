#version 430 core

// implicit inputs
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;

// a workgroup for each pixel
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// the divergence output buffer
layout (binding = 0, r32f) uniform image2D uni_buffer;

// the source vector field
layout (binding = 1, rg32f) uniform image2D uni_field_V;

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

    float hori = imageLoad(uni_field_V, ivec2(i+1,j)).r - imageLoad(uni_field_V, ivec2(i-1,j)).r;
    float vert = imageLoad(uni_field_V, ivec2(i,j+1)).g - imageLoad(uni_field_V, ivec2(i,j-1)).g;

    float div = hori + vert;
    imageStore(uni_buffer, ij, vec4(div, 0.0, 0.0, 0.0));
}

