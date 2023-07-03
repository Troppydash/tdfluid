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
layout (binding = 1, rg32f) uniform image2D uni_velocity_buffer;

layout (binding = 2, r32f) uniform image2D uni_pressure;
layout (binding = 3, r32f) uniform image2D uni_pressure_buffer;

layout (binding = 4, r32f) uniform image2D uni_density;

layout (binding = 5, rg32f) uniform image2D uni_normals;

layout (binding = 6, r32f) uniform image2D uni_mask;


void main()
{
    ivec2 ij = ivec2(gl_GlobalInvocationID.xy);

    float mask = imageLoad(uni_mask, ij).r;
    // <0.5 are fluid cells
    if (mask < 0.5)
    {
        imageStore(uni_pressure_buffer, ij, imageLoad(uni_pressure, ij));
        imageStore(uni_velocity_buffer, ij, imageLoad(uni_velocity, ij));

        return;
    }

    vec2 normal = imageLoad(uni_normals, ij).rg;
    vec2 target = vec2(ij) + normal / length(normal);
    vec2 anchor = floor(target);
    vec2 delta = target - anchor;

    ivec2 base = ivec2(anchor);

    // bilinear interpolate using target and anchor

    // static boundaries enforces zero normal directional derivatives along
    // the boundaries

    // set equal pressure, to reduce pressure induced velocities
    float P00 = imageLoad(uni_pressure, base).r;
    float P10 = imageLoad(uni_pressure, base+ivec2(1, 0)).r;
    float P01 = imageLoad(uni_pressure, base+ivec2(0, 1)).r;
    float P11 = imageLoad(uni_pressure, base+ivec2(1, 1)).r;

    float top_pressure = P01 + (P11-P01) * delta.x;
    float bottom_pressure = P00 + (P10-P00) * delta.x;
    float target_pressure = bottom_pressure + (top_pressure - bottom_pressure) * delta.y;

    imageStore(uni_pressure_buffer, ij, vec4(target_pressure, 0.0, 0.0, 0.0));


    // set inverse velocity
    vec2 V00 = imageLoad(uni_velocity, base).rg;
    vec2 V10 = imageLoad(uni_velocity, base+ivec2(1, 0)).rg;
    vec2 V01 = imageLoad(uni_velocity, base+ivec2(0, 1)).rg;
    vec2 V11 = imageLoad(uni_velocity, base+ivec2(1, 1)).rg;

    vec2 top_velocity = V01 + (V11-V01) * delta.x;
    vec2 bottom_velocity = V00 + (V10-V00) * delta.x;
    vec2 target_velocity = bottom_velocity + (top_velocity - bottom_velocity) * delta.y;

    imageStore(uni_velocity_buffer, ij, vec4(-target_velocity, 0.0, 0.0));

    // set zero density
    imageStore(uni_density, ij, vec4(0.0, 0.0, 0.0, 0.0));



}

