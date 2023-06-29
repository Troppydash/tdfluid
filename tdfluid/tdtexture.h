#pragma once



#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vector>
#include <iostream>

namespace td
{
	class texture
	{
	public:
		texture() {}

		// Load a RGB image texture
		texture &load(const char *filename)
		{
			// flip axis
			stbi_set_flip_vertically_on_load(true);

			// load texture image
			int width, height, channels;
			unsigned char *data = stbi_load(
				filename,
				&width,
				&height,
				&channels,
				0
			);

			if (!data)
			{
				std::cout << "Failed to load textures form file: " << filename << std::endl;
				throw std::runtime_error("");
			}

			// allocate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// set texture wrapping and filtering options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// the line tells OpenGL that our pixel is 1 byte aligned
			// meaning a tightly packed image data
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// push data to the texture
			// and generate mipmap
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);

			// save id
			m_textures.push_back(texture);
		}


		void use() const
		{
			for (int i = 0; i < m_textures.size(); ++i)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, m_textures[i]);
			}
		}

		void end()
		{
			for (auto &texture : m_textures)
			{
				glDeleteTextures(1, &texture);
			}
		}

	private:
		std::vector<unsigned int> m_textures;
	};
}
