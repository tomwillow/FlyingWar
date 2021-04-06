#include "SceneCover.h"

#include "SceneController.h"
#include "ResMacro.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

SceneCover::SceneCover(SceneController* in_controller):
	Scene(in_controller),
	texBackground(TEXTURE_COVER)
{
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

void SceneCover::Render(float dt)
{
	this->DrawBackground();
}

void SceneCover::OnKey(int key, int action)
{
	if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE)
	{
		controller->GoLevel(0);
	}
}
