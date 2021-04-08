#include "Scene.h"

#include "ResMacro.h"
#include "ResValue.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

Scene::Scene(SceneController* in_controller) :
	W(DEFAULT_WIDTH),H(DEFAULT_HEIGHT),
	controller(in_controller),
	spiritShader(SHADER_SPIRIT_VERTEX, SHADER_SPIRIT_FRAGMENT),
	renderer(spiritShader)
{
	OnSize(W, H);

	spiritShader.UseProgram();
	mat4 view(1.0f);
	spiritShader.Uniform("view", view);
}


void Scene::OnSize(int in_width, int in_height)
{
	W = in_width;
	H = in_height;

	spiritShader.UseProgram();
	mat4 projection(1.0f);
	projection = ortho(0.0f, (float)W, 0.0f, (float)H);

	spiritShader.Uniform("projection", projection);
}
