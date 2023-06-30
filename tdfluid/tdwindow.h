#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

namespace td
{
	class window;
	class window_object
	{
	public:
		virtual void start(window &window) = 0;
		virtual void update(float dt) = 0;
		virtual void render() = 0;
		virtual void end() = 0;
	};

	struct window_settings
	{
		int width;
		int height;
		std::string title;

	};

	class window
	{
	public:
		window()
		{}

		window(window_settings settings)
			: m_settings(settings)
		{}

		void include(window_object *object)
		{
			m_objects.push_back(object);
		}

		void start()
		{
			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			// create window
			m_window = glfwCreateWindow(
				m_settings.width,
				m_settings.height,
				m_settings.title.c_str(),
				nullptr,
				nullptr
			);
			glfwMakeContextCurrent(m_window);

			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

			glViewport(0, 0, m_settings.width, m_settings.height);
			//glfwSwapInterval(1);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (auto &object : m_objects)
			{
				object->start(*this);
			}
		}

		void main()
		{
			double last_time = glfwGetTime();

			while (!glfwWindowShouldClose(m_window))
			{
				double time = glfwGetTime();
				double dt = time - last_time;
				last_time = time;

				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				for (auto &object : m_objects)
				{
					object->update((float)dt);
				}

				for (auto &object : m_objects)
				{
					object->render();
				}

				glfwPollEvents();
				glfwSwapBuffers(m_window);
			}
		}


		void end()
		{
			for (auto &object : m_objects)
			{
				object->end();
			}

			glfwDestroyWindow(m_window);
			glfwTerminate();
		}

		GLFWwindow *get_window()
		{
			return m_window;
		}

	private:
		window_settings m_settings = {
			750,
			750,
			"tdfluid"
		};

		// non-portable members
		GLFWwindow *m_window = nullptr;

		std::vector<window_object *> m_objects;
	};
}
