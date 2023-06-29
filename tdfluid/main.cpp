#include <glad/glad.h>

#include "tdwindow.h"
#include "tdmesh.h"
#include "tdshader.h"
#include "tdtexture.h"
#include "tdmeshes.h"

#include "tdui.h"

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

		// load shaders
		m_shader.load("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");
		m_jacobi.load("resources/shaders/jacobi.glsl", m_width, m_height);
		m_copy.load("resources/shaders/copy.glsl", m_width, m_height);
		m_merge.load("resources/shaders/merge.glsl", m_width, m_height);

		// load textures
		m_pressure.load_empty_r32f(m_width, m_height);
		m_pressure_buffer.load_empty_r32f(m_width, m_height);

		m_density.load_empty_r32f(m_width, m_height);
		m_density_buffer.load_empty_r32f(m_width, m_height);
		m_density_last.load_empty_r32f(m_width, m_height);
		m_density_source.load_empty_r32f(m_width, m_height);

		m_mask.load_empty_r32f(m_width, m_height);


		// center blob in density
		std::vector<float> buffer;
		for (int x = 0; x < m_width; ++x)
		{
			for (int y = 0; y < m_height; ++y)
			{
				float value = 0.0f;
				/*if ((x - 250) * (x - 250) + (y - 250) * (y - 250) < 25 * 25)
				{
					value = 1.0f;
				}*/
				if ((x-500) == (y- 500) || (x- 500) == -(y- 500))
				{
					value = 1.0f;
				}
				buffer.push_back(value);
			}
		}
		m_density.upload_r32f(buffer);
		m_density_source.upload_r32f(buffer);

		// border with mask
		buffer.clear();
		for (int x = 0; x < m_width; ++x)
		{
			for (int y = 0; y < m_height; ++y)
			{
				float value = 0.0f;
				if (x == 0 || y == 0 || x == m_width - 1 || y == m_height - 1)
				{
					value = 1.0f;
				}
				buffer.push_back(value);
			}
		}
		m_mask.upload_r32f(buffer);

	}


	void update(float dt) override
	{
		// fuck it this looks right

		// add source
		run_merge(m_density_source, m_density);

		// recalculate normals and rebake mask
		// TODO

		// boundary conditions
		// TODO

		// run projection
		// TODO

		// run diffusion
		run_diffusion(dt);

		// run advection
		// TODO
	}

	void render() override
	{
		m_density.use();
		m_shader.use();
		m_mesh.render();
	}

	void end() override
	{
		m_shader.end();
		m_mesh.end();
	}

	/// FLUID DYNAMICS ALGORITHMS ///

	void run_merge(td::texture &from, td::texture &to)
	{
		from.use_image(0);
		to.use_image(1);

		m_merge.use();
		m_merge.execute();
	}

	void run_copy(td::texture &from, td::texture &to)
	{
		from.use_image(0);
		to.use_image(1);

		m_copy.use();
		m_copy.execute();
	}

	void run_diffusion(float dt)
	{
		float a = 1 / (1 + 4 * m_diffusion_rate * dt);
		float b = m_diffusion_rate * dt;
		float c = 1;

		// copy m_density to m_density_last
		run_copy(m_density, m_density_last);

		// we always uses these

		// V = rho'
		m_density.use_image(0);
		// output here
		m_density_buffer.use_image(1);
		// W = rho
		m_density_last.use_image(2);
		// mask
		m_mask.use_image(3);
		// jacobi cosntants
		m_jacobi.set_uniform_scalar("a", a);
		m_jacobi.set_uniform_scalar("b", b);
		m_jacobi.set_uniform_scalar("c", c);

		// iterate
		for (int i = 0; i < 5; ++i)
		{
			m_jacobi.use();
			m_jacobi.execute();

			// copy buffer to density
			run_copy(m_density_buffer, m_density);
		}

		// now m_density contains the new diffused density
	}

	void run_projection()
	{
		// compute pressure field
		float a = 1 / 4;
		float b = 1;
		float c = -1;

	}

private:
	// settings
	int m_width = 1000;
	int m_height = 1000;

	float m_diffusion_rate = 1000.0f;

	// references
	td::mesh m_mesh;

	td::graphics_shader m_shader;
	td::compute_shader m_jacobi;
	td::compute_shader m_copy;
	td::compute_shader m_merge;

	td::texture m_pressure;
	td::texture m_pressure_buffer;

	// the correct density field
	td::texture m_density;
	// the density field used in iteration
	td::texture m_density_buffer;
	// the last density field
	td::texture m_density_last;
	// the target density field (source)
	td::texture m_density_source;

	td::texture m_mask;


};


void simulation()
{
	td::window window;

	fluid_object object;
	window.include(&object);

	td::ui ui;
	window.include(&ui);

	window.start();

	window.main();

	window.end();
}


int main(int argc, char *argv)
{
	simulation();
	return 0;
}