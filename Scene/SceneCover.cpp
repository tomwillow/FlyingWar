#include "SceneCover.h"

#include "SceneController.h"
#include "ResMacro.h"

#include "tstring.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

using namespace std;
using namespace glm;

SceneCover::SceneCover(SceneController* in_controller):
	Scene(in_controller),
	texBackground(TEXTURE_COVER)
{
	glfwSetInputMode(controller->glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}


void SceneCover::DrawBackground()
{
	renderer.Draw(texBackground, vec2(0, 0), vec2(W, H));
}

void SceneCover::DrawBackground(SpiritRenderer& in_renderer)
{
	in_renderer.Draw(texBackground, vec2(0, 0), vec2(W, H));
}

void SceneCover::DrawBackground(SpiritRenderer& in_renderer, glm::vec2 pos, glm::vec2 size)
{
	in_renderer.Draw(texBackground, pos, size);
}

void SceneCover::BeforeGLClear(float dt)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBackground;

	// We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
	// Based on your use case you may want one of the other.
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	if (ImGui::Begin("Fullscreen window", NULL, flags))
	{
		auto sz = ImGui::GetWindowSize();
		float button_width = ImGui::GetFontSize() * 6;

		ImGui::SetCursorPos({ (sz.x - button_width) / 2,sz.y / 2 });
		if (ImGui::Button(u8"开始游戏", { button_width,0 }))
		{
			controller->GoLevel(0);
		}

		ImGui::SetCursorPosX((sz.x - button_width) / 2);
		if (ImGui::Button(u8"排行榜", { button_width,0 }))
		{
			ImGui::OpenPopup(u8"排行榜");
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(u8"排行榜", NULL, ImGuiWindowFlags_AlwaysAutoResize))//ImGuiWindowFlags_NoCollapse
		{

			auto &record = controller->billboard.GetRecord();
			if (record.empty())
			{
				ImGui::Text(u8"无记录");
			}
			else
			{
				int i = 1;
				ImGui::Text(u8"排名\t姓名\t分数");
				for (auto it = record.rbegin(); it != record.rend(); ++it)
				{
					string name = it->second;
					int score = it->first;
					string s = to_string(i) + "\t" + string2utf8(name) + "\t" + to_string(score);
					//ImGui::Text("%d\t%s\t%d", i, string2utf8(name), score);
					ImGui::Text(s.c_str());
					i++;
				}
			}

			if (ImGui::Button(u8"确定"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		ImGui::SetCursorPosX((sz.x - button_width) / 2);
		if (ImGui::Button(u8"操作说明", { button_width,0 }))
		{
			ImGui::OpenPopup(u8"操作说明");
		}

		// Always center this window when appearing
		//ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(u8"操作说明", NULL, ImGuiWindowFlags_AlwaysAutoResize))//ImGuiWindowFlags_NoCollapse
		{
			ImGui::Text(u8"鼠标上下移动：控制飞机俯仰");
			ImGui::Text(u8"鼠标左右移动：控制飞机偏转");
			ImGui::Text(u8"键盘W/S键：加速/减速");
			ImGui::Text(u8"空格：开火");
			if (ImGui::Button(u8"确定"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		ImGui::SetCursorPosX((sz.x - button_width) / 2);
		if (ImGui::Button(u8"退出", { button_width,0 }))
		{
			glfwSetWindowShouldClose(controller->glfw_window, true);
		}

	}
	ImGui::End();

	// Rendering
	ImGui::Render();
}

void SceneCover::Render(float dt)
{
	this->DrawBackground();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneCover::OnKey(int key, int action)
{
	if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE)
	{
		controller->GoLevel(0);
	}
}
