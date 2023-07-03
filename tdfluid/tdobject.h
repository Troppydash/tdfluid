#pragma once

#include <vector>
#include <tuple>

#include "tdwindow.h"
#include "tdmesh.h"
#include "tdshader.h"
#include "tdtexture.h"
#include "tdmeshes.h"



namespace td
{
	

	class fluid_static_boundary
	{
	public:
		virtual bool is_within(int x, int y) const = 0;
		virtual std::pair<float, float> compute_normal(int x, int y) const = 0;
	};

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
			m_copy_rg.load("resources/shaders/copy_rg.glsl", m_width, m_height);

			m_merge.load("resources/shaders/merge.glsl", m_width, m_height);
			m_merge_rg.load("resources/shaders/merge_rg.glsl", m_width, m_height);

			m_divergence.load("resources/shaders/divergence.glsl", m_width, m_height);
			m_project.load("resources/shaders/project.glsl", m_width, m_height);

			m_advect.load("resources/shaders/advect.glsl", m_width, m_height);
			m_advect_rg.load("resources/shaders/advect_rg.glsl", m_width, m_height);

			m_static_boundary.load("resources/shaders/static_boundary.glsl", m_width, m_height);


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
			for (int i = 0; i < (m_first ? 70 : 6); ++i)
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
			m_density.use(0);
			m_mask.use(1);
			m_shader.use();
			m_mesh.render();
		}

		void end() override
		{
			m_shader.end();
			m_mesh.end();

			m_jacobi.end();
			m_copy.end();
			m_copy_rg.end();
			m_merge.end();
			m_merge_rg.end();
			m_divergence.end();
			m_project.end();
			m_advect.end();
			m_static_boundary.end();
			m_advect_rg.end();

			m_velocity.end();
			m_velocity_source.end();
			m_velocity_buffer.end();
			m_velocity_divergence.end();

			m_pressure.end();
			m_pressure_buffer.end();
			m_density.end();
			m_density_buffer.end();
			m_density_last.end();
			m_density_source.end();

			m_normal.end();
			m_mask.end();
		}

		void add_object(td::fluid_static_boundary *object)
		{
			m_objects.push_back(object);
		}

		/// FLUID DYNAMICS ALGORITHMS ///

		void initialize()
		{
			std::vector<float> buffer;
			for (int y = 0; y < m_height; ++y)
			{
				for (int x = 0; x < m_width; ++x)
				{
					float value = 0.0f;
					if (abs(y - 256.0) <= 10.0 && x < 5.0f)
						value = 1.7f;
					if (abs(x - 256.0) <= 10.0 && y < 5.0f)
						value = 1.5f;
					buffer.push_back(value);
				}
			}
			m_density.upload_r32f(buffer);
			m_density_source.upload_r32f(buffer);

			// velocity
			buffer.clear();
			for (int y = 0; y < m_height; ++y)
			{
				for (int x = 0; x < m_width; ++x)
				{
					if (abs(y - 256.0f) < 5.0f && x < 50.0f)
					{
						buffer.push_back(700.0f);
						buffer.push_back(0.0f);
					}
					else if (abs(x - 256.0f) < 5.0f && y < 50.0f)
					{
						buffer.push_back(0.0f);
						buffer.push_back(700.0f);
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


			// static boundaries
			std::vector<float> cells(m_width * m_height);
			std::vector<float> normals(2 * m_width * m_height);

			for (int y = 0; y < m_height; ++y)
			{
				for (int x = 0; x < m_width; ++x)
				{
					float cell = 0.0f;
					float nx = 0.0f;
					float ny = 0.0f;
					for (td::fluid_static_boundary *object : m_objects)
					{
						if (!object->is_within(x, y))
						{
							continue;
						}

						cell = 1.0f;
						auto normal = object->compute_normal(x, y);
						nx = normal.first;
						ny = normal.second;
					}

					cells[y * m_width + x] = cell;
					normals[2 * (y * m_width + x)] = nx;
					normals[2 * (y * m_width + x) + 1] = ny;
				}
			}
			m_mask.upload_r32f(cells);
			m_normal.upload_rg32f(normals);
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

			// iterate
			int seesaw = 0;
			for (int i = 0; i < 6; ++i)
			{
				m_jacobi.use();
				m_jacobi.execute();

				// copy buffer to density
				m_density.use_image(1 - seesaw);
				m_density_buffer.use_image(seesaw);
				seesaw = 1 - seesaw;
				//run_copy(m_density_buffer, m_density);
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

			int seesaw = 0;
			for (int i = 0; i < 8; ++i)
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

		int get_width() const
		{
			return m_width;
		}

		int get_height() const
		{
			return m_height;
		}
	private:
		// settings
		int m_width = 512;
		int m_height = 512;

		float m_diffusion_rate = 200.0f;

		// objects
		std::vector<td::fluid_static_boundary *> m_objects;

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

		td::texture m_density;
		td::texture m_density_buffer;
		td::texture m_density_last;
		td::texture m_density_source;

		td::texture m_normal;
		td::texture m_mask;


		bool m_first = true;
	};

	class fluid_ball : public fluid_static_boundary
	{
	public:
		fluid_ball(int x, int y, int radius)
			: m_x(x), m_y(y), m_radius(radius)
		{}

		bool is_within(int x, int y) const override
		{
			return powf(x - m_x, 2) + powf(y - m_y, 2) <= powf(m_radius, 2);
		}

		std::pair<float, float> compute_normal(int x, int y) const override
		{
			return { x - m_x, y - m_y };
		}

	protected:
		int m_x;
		int m_y;
		int m_radius;
	};


	class fluid_closed_border : public fluid_static_boundary
	{
	public:
		fluid_closed_border(int width, int height)
			: m_width(width), m_height(height)
		{}


		bool is_within(int x, int y) const override
		{
			if (x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1)
				return true;
			return false;
		}
		
		std::pair<float, float> compute_normal(int x, int y) const override
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

			return { nx, ny };
		}

	protected:
		int m_width;
		int m_height;
	};

}