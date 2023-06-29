#version 430 core

out vec4 out_color;

in vec2 in_tex;
layout (binding = 0) uniform sampler2D uni_texture;

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
    float brightness = texture(uni_texture, in_tex).r;

    if (brightness < 0.01) {
        discard;
    }

    vec3 color = rainbow(brightness);
    out_color = vec4(color, 1.0);
}