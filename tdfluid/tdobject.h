#pragma once

#include <vector>

#include "tdmesh.h"
#include "tdmaterial.h"


namespace td
{
	class object
	{
	public:
		object(
			td::mesh &mesh,
			td::material &material
		) : m_meshes(mesh), m_materials(material)
		{}

		void render()
		{
			m_materials.use();
			m_meshes.render();
		}

	private:
		td::mesh &m_meshes;
		td::material &m_materials;
	};
}