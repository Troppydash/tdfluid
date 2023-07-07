#define _CRT_SECURE_NO_WARNINGS


#include <glad/glad.h>

#include "tdwindow.h"
#include "tdobject.h"

#include "tdui.h"

#include "tdnoise.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

void build_textures()
{
	// build perlin noise texture
	const int width = 512;
	const int height = 512;
	unsigned char *data = new unsigned char[width * height * 3];

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float sample_x = (float)x / 200.0f + 10.0f;
			float sample_y = (float)y / 200.0f + 10.0f;
			//float value = noise.sample(sample_x, sample_y);
			float value = stb_perlin_turbulence_noise3(sample_x, sample_y, 0.0f, 2.0, 0.5, 6);

			unsigned int brightness = (value) * 256.0f;

			unsigned int index = (y * width + x) * 3;
			data[index] = brightness;
			data[index + 1] = brightness;
			data[index + 2] = brightness;
		}
	}

	stbi_write_jpg("noise.jpg", width, height, 3, data, 100);

	delete[] data;
}

void simulation()
{
	td::window window;

	// create fluid field
	td::fluid_object object;

	// create static boundaries
	td::fluid_closed_border *border = new td::fluid_closed_border(object.get_width(), object.get_height());
	object.add_object(border);

	td::fluid_ball *ball = new td::fluid_ball(200, 200, 100);
	object.add_object(ball);

	// add fluid field to window
	window.include(&object);

	// and add ui
	td::ui ui;
	window.include(&ui);

	window.start();
	window.main();

	// cleanup
	delete border;
	delete ball;
	window.end();
}


int main(int argc, char *argv)
{
	simulation();
	//build_textures();
	return 0;
}