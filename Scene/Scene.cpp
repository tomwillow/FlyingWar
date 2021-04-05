#include "Scene.h"

#include "ResMacro.h"
#include "ResValue.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

Scene::Scene(SceneController* in_controller) :
	W(DEFAULT_WIDTH),H(DEFAULT_HEIGHT),sceneWidthMulti(2.0f),
	controller(in_controller),
	shader(SHADER_SPIRIT_VERTEX, SHADER_SPIRIT_FRAGMENT),
	renderer(shader)
{
	OnSize(W, H);

	shader.UseProgram();
	mat4 view(1.0f);
	shader.Uniform("view", view);
}


void Scene::OnSize(int in_width, int in_height)
{
	W = in_width;
	H = in_height;

	shader.UseProgram();
	mat4 projection(1.0f);
	projection = ortho(0.0f, (float)W, 0.0f, (float)H);

	shader.Uniform("projection", projection);
}
