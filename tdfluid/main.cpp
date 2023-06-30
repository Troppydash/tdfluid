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
		m_divergence.load("resources/shaders/divergence.glsl", m_width, m_height);
		m_project.load("resources/shaders/project.glsl", m_width, m_height);
		m_advect.load("resources/shaders/advect.glsl", m_width, m_height);
		m_static_boundary.load("resources/shaders/static_boundary.glsl", m_width, m_height);

		m_copy_rg.load("resources/shaders/copy_rg.glsl", m_width, m_height);
		m_merge_rg.load("resources/shaders/merge_rg.glsl", m_width, m_height);

		m_advect_rg.load("resources/shaders/advect_rg.glsl", m_width, m_height);

		// load textures
		m_velocity.load_empty_rg32f(m_width, m_height);
		m_velocity_source.load_empty_rg32f(m_width, m_height);
		m_velocity_buffer.load_empty_rg32f(m_width, m_height);
		m_velocity_divergence.load_empty_r32f(m_width, m_height);

		m_pressure.load_empty_r32f(m_width, m_height);
		m_pressure_buffer.load_empty_r32f(m_width, m_height);

		m_density.load_empty_r32f(m_width, m_height);
		m_density_buffer.load_empty_r32f(m_width, m_height);
		m_density_last.load_empty_r32f(m_width, m_height);
		m_density_source.load_empty_r32f(m_width, m_height);

		m_mask.load_empty_r32f(m_width, m_height);
		m_normal.load_empty_rg32f(m_width, m_height);

		initialize();
	}


	void update(float dt) override
	{
		// fuck it this looks right

		// add source
		//if (((float)rand() / RAND_MAX) > 0.9f)
		run_merge(m_density_source, m_density);
		
		run_merge_rg(m_velocity_source, m_velocity);

		// recalculate normals and rebake mask
		// TODO

		// boundary conditions

		// run projection
		for (int i = 0; i < (m_first ? 70 : 10); ++i)
		{
			run_boundary();
			run_projection();
		}

		// run diffusion
		run_diffusion(dt);

		// run advection
		run_advection(dt);


		m_first = false;
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

	void initialize()
	{
		// center blob in density
		std::vector<float> buffer;
		for (int y = 0; y < m_height; ++y)
		{
			for (int x = 0; x < m_width; ++x)
			{
				float value = 0.0f;
				if (abs(y - 123.0) <= 2.0 && x < 5.0f)
					value = 2.2f;
				if (abs(x - 123.0) <= 2.0 && y < 5.0f)
					value = 1.5f;
				buffer.push_back(value);
			}
		}
		m_density.upload_r32f(buffer);
		m_density_source.upload_r32f(buffer);

		// border with mask
		buffer.clear();
		for (int y = 0; y < m_height; ++y)
		{
			for (int x = 0; x < m_width; ++x)
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

		// compute normals for border
		buffer.clear();
		for (int y = 0; y < m_height; ++y)
		{
			for (int x = 0; x < m_width; ++x)
			{
				float nx = 0.0f;
				float ny = 0.0f;
				if (x == 0)
				{
					nx = 1.0f;
				}
				if (x == m_width - 1)
				{
					nx = -1.0f;
				}
				if (y == 0)
				{
					ny = 1.0f;
				}
				if (y == m_height - 1)
				{
					ny = -1.0f;
				}

				// normalize
				float length = sqrt(powf(nx, 2) + powf(ny, 2));
				if (length < 0.01f)
				{
					// zero normal
					buffer.push_back(0.0f);
					buffer.push_back(0.0f);
				}
				else
				{
					// normalized normal
					buffer.push_back(nx);
					buffer.push_back(ny);
				}
			}
		}
		m_normal.upload_rg32f(buffer);


		// zeroing the pressure
		buffer.clear();
		for (int y = m_height - 1; y >= 0; --y)
		{
			for (int x = 0; x < m_width; ++x)
			{
				buffer.push_back(0.0f);
			}
		}
		m_pressure.upload_r32f(buffer);


		// velocity
		buffer.clear();
		for (int y = 0; y < m_height; ++y)
		{
			for (int x = 0; x < m_width; ++x)
			{
				// velocity of [1.0, 0.0]
				if (abs(y - 123.0f) < 5.0f && x < 10.0f)
				{
					buffer.push_back(150.0f);
					buffer.push_back(0.0f);
				}
				else if (abs(x - 123.0f) < 5.0f && y < 10.0f)
				{
					buffer.push_back(0.0f);
					buffer.push_back(150.0f);
				}
				else
				{
					buffer.push_back(0.0f);
					buffer.push_back(0.0f);
				}
			}
		}
		m_velocity.upload_rg32f(buffer);
		m_velocity_source.upload_rg32f(buffer);
	}

	void run_merge(td::texture &from, td::texture &to)
	{
		from.use_image(0);
		to.use_image(1);

		m_merge.use();
		m_merge.execute();
	}

	void run_merge_rg(td::texture &from, td::texture &to)
	{
		from.use_image<2>(0);
		to.use_image<2>(1);

		m_merge_rg.use();
		m_merge_rg.execute();
	}

	void run_copy(td::texture &from, td::texture &to)
	{
		from.use_image(0);
		to.use_image(1);

		m_copy.use();
		m_copy.execute();
	}

	void run_copy_rg(td::texture &from, td::texture &to)
	{
		from.use_image<2>(0);
		to.use_image<2>(1);

		m_copy_rg.use();
		m_copy_rg.execute();
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
		for (int i = 0; i < 10; ++i)
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
		// compute div V
		m_velocity_divergence.use_image(0);
		m_velocity.use_image<2>(1);
		m_mask.use_image(2);

		m_divergence.use();
		m_divergence.execute();

		// compute pressure field
		float a = 1.0f / 4.0f;
		float b = 1.0f;
		float c = -1.0f;

		// warm start the pressure buffer
		// run_copy(m_pressure, m_pressure_buffer);

		m_pressure.use_image(0);
		m_pressure_buffer.use_image(1);
		m_velocity_divergence.use_image(2);
		m_mask.use_image(3);

		m_jacobi.set_uniform_scalar("a", a);
		m_jacobi.set_uniform_scalar("b", b);
		m_jacobi.set_uniform_scalar("c", c);

		int seesaw = 0;
		for (int i = 0; i < 10; ++i)
		{
			m_jacobi.use();
			m_jacobi.execute();

			// toggle buffers
			// run_copy(m_pressure_buffer, m_pressure);
			m_pressure.use_image(1 - seesaw);
			m_pressure_buffer.use_image(seesaw);
			seesaw = 1 - seesaw;
		}

		// we now have m_pressure set to the correct pressure
		//return;
		// update velocity
		m_pressure.use_image(0);
		m_velocity.use_image<2>(1);
		m_mask.use_image(2);

		m_project.use();
		m_project.execute();

		// m_velocity is then updated
	}

	void run_advection(float dt)
	{
		m_velocity.use_image<2>(0);
		m_density.use_image(1);
		m_density_buffer.use_image(2);
		m_mask.use_image(3);

		m_advect.set_uniform_scalar("dt", dt);

		m_advect.use();
		m_advect.execute();

		run_copy(m_density_buffer, m_density);

		// also advect the velocity
		m_velocity.use_image<2>(0);
		m_velocity_buffer.use_image<2>(1);
		m_mask.use_image(2);

		m_advect_rg.set_uniform_scalar("dt", dt);

		m_advect_rg.use();
		m_advect_rg.execute();

		run_copy_rg(m_velocity_buffer, m_velocity);
	}

	void run_boundary()
	{
		// boundary conditions on velocity, pressure, and density
		m_velocity.use_image<2>(0);
		m_velocity_buffer.use_image<2>(1);

		m_pressure.use_image(2);
		m_pressure_buffer.use_image(3);

		m_density.use_image(4);

		m_normal.use_image<2>(5);
		m_mask.use_image(6);

		m_static_boundary.use();
		m_static_boundary.execute();

		// copy buffers
		run_copy_rg(m_velocity_buffer, m_velocity);
		run_copy(m_pressure_buffer, m_pressure);
	}

private:
	// settings
	int m_width = 256;
	int m_height = 256;

	float m_diffusion_rate = 1000.0f;

	// references
	td::mesh m_mesh;

	td::graphics_shader m_shader;
	td::compute_shader m_jacobi;
	td::compute_shader m_copy;
	td::compute_shader m_copy_rg;
	td::compute_shader m_merge;
	td::compute_shader m_divergence;
	td::compute_shader m_project;
	td::compute_shader m_advect;
	td::compute_shader m_static_boundary;
	td::compute_shader m_merge_rg;
	td::compute_shader m_advect_rg;

	// texture buffers
	td::texture m_velocity;
	td::texture m_velocity_source;
	td::texture m_velocity_buffer;
	td::texture m_velocity_divergence;

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

	td::texture m_normal;
	td::texture m_mask;

	bool m_first = true;
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