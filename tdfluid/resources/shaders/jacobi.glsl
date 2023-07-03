#version 430 core

// implicit inputs
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;

// a workgroup for each pixel
layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// the actual field to operate on
layout (binding = 0, r32f) uniform image2D uni_field_V;

// the temporary buffer for the jacobi iteration
layout (binding = 1, r32f) uniform image2D uni_buffer;

layout (binding = 2, r32f) uniform image2D uni_field_W;

// only operate on cells with `mask[x,y] < 0.5`
layout (binding = 3, r32f) uniform image2D uni_mask;

// jacobi iteration constants
uniform float a;
uniform float b;

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

    float left = imageLoad(uni_field_V, ivec2(i-1,j)).r;
    float right = imageLoad(uni_field_V, ivec2(i+1,j)).r;
    float bottom = imageLoad(uni_field_V, ivec2(i,j-1)).r;
    float top = imageLoad(uni_field_V, ivec2(i,j+1)).r;

    float w = imageLoad(uni_field_W, ij).r;

    float result = a * (b * (left + right + bottom + top) + w);
    imageStore(uni_buffer, ij, vec4(result, 0.0, 0.0, 0.0));
}

