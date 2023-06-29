#pragma once


#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "tdwindow.h"


namespace td
{
	class ui : public window_object
	{
	public:
		ui() {}

		void start(window &window) override
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGui::StyleColorsDark();

			ImGui_ImplGlfw_InitForOpenGL(window.get_window(), true);
			ImGui_ImplOpenGL3_Init("#version 430");

		}

		void render() override
		{
			bool show_stats = true;

			ImGuiIO &io = ImGui::GetIO();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Stats", &show_stats, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::Text("Fps: %.2f", io.Framerate);
			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		}

		void update(float dt) override
		{}

		void end() override
		{}

	private:
	};
}

