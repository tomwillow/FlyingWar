#include "PureRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

PureRenderer::PureRenderer(TShader& in_shader) :
	SpiritRenderer(in_shader)
{
}

void PureRenderer::Draw(glm::vec4 color, glm::vec2 position, glm::vec2 size, float deg)
{
	SetModelMatrix(position, size, deg);

	shader.Uniform("color", color);

	spirit.Bind();
	spirit.DrawTriangles();
}

void PureRenderer::DrawCenter(glm::vec4 color, glm::vec2 center, glm::vec2 size, float deg)
{
	SetCenterModelMatrix(center, size, deg);

	shader.Uniform("color", color);

	spirit.Bind();
	spirit.DrawTriangles();
}
