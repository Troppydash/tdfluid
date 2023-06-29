#pragma once

namespace td_meshes
{
	const float plane[] = {
		// top left
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		// top right
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		// bottom right
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		// bottom left
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
	};
	const unsigned int plane_indices[] = {
		0, 1, 2,
		0, 2, 3
	};


}