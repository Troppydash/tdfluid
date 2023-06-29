#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <iostream>
#include <optional>

#include "tdtexture.h"


static std::string readFile(const char *filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}

	std::string content = "";

	std::string line;
	while (std::getline(file, line))
	{
		content += line + "\n";
	}

	file.close();

	return content;
}



namespace td
{
	class shader
	{
	public:
		shader() {};

		shader &load(const char *vertex_file, const char *fragment_file)
		{
			std::string vertex = readFile(vertex_file);
			std::string fragment = readFile(fragment_file);

			const char *vertexSource = vertex.c_str();
			const char *fragmentSource = fragment.c_str();

			int success;
			char infoLog[512];

			// compile the vertex shader
			unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexSource, nullptr);
			glCompileShader(vertexShader);

			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);

				std::cout << "Failed to create the vertex shader\n" << infoLog << std::endl;
				throw std::runtime_error("");
			}

			// compile the fragment shader
			unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
			glCompileShader(fragmentShader);

			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);

				std::cout << "Failed to create the fragment shader\n" << infoLog << std::endl;
				throw std::runtime_error("");
			}

			// link to shader program
			m_program = glCreateProgram();
			glAttachShader(m_program, vertexShader);
			glAttachShader(m_program, fragmentShader);
			glLinkProgram(m_program);

			// check link status
			glGetProgramiv(m_program, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(m_program, 512, nullptr, infoLog);

				std::cout << "Failed to link the shader program\n" << infoLog << std::endl;
				throw std::runtime_error("");
			}

			// we are done with the vertex and fragment shaders
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			return *this;
		}

		void use() const
		{
			glUseProgram(m_program);
		}

		void set_uniform_scalar(const char *name, float data)
		{
			int location = find_uniform(name);

			use();
			glUniform1f(location, data);
		}

		template <unsigned int N = 4>
		void set_uniform_matrix(const char *name, const float *data)
		{
			int location = find_uniform(name);

			use();
			switch (N)
			{
			case 4:
				glUniformMatrix4fv(location, 1, GL_FALSE, data);
			default:
				std::cout << "Shader: Unable to set the uniform of a matrix with size outside [1, 4]" << std::endl;
				throw std::runtime_error("");
			}
		}

		void end()
		{
			glDeleteProgram(m_program);
		}

	private:
		int find_uniform(const char *name) const
		{
			int location = glGetUniformLocation(m_program, name);
			if (location == -1)
			{
				std::cout << "Failed to find the shader uniform location: " << name << std::endl;
				throw std::runtime_error("");
			}
			return location;
		}

	private:
		unsigned int m_program = -1;
	};


	class compute_shader
	{

	};
}
