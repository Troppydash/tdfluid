#version 430 core

out vec4 out_color;

in vec2 in_tex;
layout (binding = 0) uniform sampler2D uni_texture;
// layout (binding = 1) uniform sampler2D uni_mask;

// rainbow the density output
vec3 rainbow(float level)
{
    level *= 5.0;
	float r = float(level <= 2.0) + float(level > 4.0) * 0.5;
	float g = max(1.0 - abs(level - 2.0) * 0.5, 0.0);
	float b = (1.0 - (level - 4.0) * 0.5) * float(level >= 4.0);
	return vec3(r, g, b);
}



void main()
{
    
    // vec2 velocity = texture(uni_texture, in_tex).rg;
    // out_color = vec4(velocity, 0.0, 1.0);
    // return;

    float brightness = texture(uni_texture, in_tex).r;

     if (brightness < 0.01) {
         discard;
     }
    //out_color = vec4(brightness, brightness, brightness, 1.0);
    //return;
    vec3 color = rainbow(brightness);
    out_color = vec4(color, brightness);
}