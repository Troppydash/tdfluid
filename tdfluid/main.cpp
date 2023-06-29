#include <glad/glad.h>

#include "tdwindow.h"
#include "tdmesh.h"
#include "tdshader.h"
#include "tdtexture.h"
#include "tdmeshes.h"

class fluid_object : public td::window_object
{
public:
	fluid_object() {}

	void start(td::window &window) override
	{
		// load mesh, shader, texture

		// load mesh
		int position_size = 5 * 4;
		const float *plane = td_meshes::plane;
		std::vector<float> positions(plane, plane + position_size);
		
		int indices_size = 6;
		const unsigned int *plane_indices = td_meshes::plane_indices;
		std::vector<unsigned int> indices(plane_indices, plane_indices + indices_size);

		m_mesh.load5(positions);
		m_mesh.load_indices(indices);

		// load texture
		m_texture.load("resources/textures/nyws.jpg");

		// load shaders
		m_shader.load("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");
	}

	void update(float dt) override
	{

	}

	void render() override
	{
		m_texture.use();
		m_shader.use();
		m_mesh.render();
	}

	void end() override
	{
		m_texture.end();
		m_shader.end();
		m_mesh.end();
	}

private:
	td::mesh m_mesh;
	td::shader m_shader;
	td::texture m_texture;
};


void simulation()
{
	td::window window;

	fluid_object object;
	window.include(&object);

	window.start();

	window.main();

	window.end();
}


int main(int argc, char *argv)
{
	simulation();
	return 0;
}