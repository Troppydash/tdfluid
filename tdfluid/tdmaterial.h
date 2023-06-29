#pragma once

#include "tdshader.h"
#include "tdtexture.h"

namespace td
{
	class material
	{
	public:
		material(
			td::shader &shader,
			td::texture &texture
		) : m_shader(shader), m_texture(texture)
		{}

		void use() const
		{
			m_texture.use();
			m_shader.use();
		}

	private:
		td::shader &m_shader;
		td::texture &m_texture;
	};
}
