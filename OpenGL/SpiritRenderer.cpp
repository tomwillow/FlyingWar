#include "SpiritRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

SpiritRenderer::SpiritRenderer(TShader& in_shader):
    shader(in_shader)
{
}

void SpiritRenderer::Draw(TTexture& tex, glm::vec2 position, glm::vec2 size, float deg)
{
	SetModelMatrix(position, size, deg);
	shader.Uniform("Tex", tex);

	spirit.Bind();
	spirit.DrawTriangles();

}

void SpiritRenderer::DrawCenter(TTexture& tex, glm::vec2 position, glm::vec2 size, float deg)
{
	SetCenterModelMatrix(position, size, deg);
	shader.Uniform("Tex", tex);

	spirit.Bind();
	spirit.DrawTriangles();
}

void SpiritRenderer::SetModelMatrix(glm::vec2 position, glm::vec2 size, float deg)
{
	shader.UseProgram();
	mat4 model(1.0f);
	model = translate(model, vec3(position,0));
	model = scale(model, vec3(size, 0));
	model = rotate(model, (float)radians(deg), vec3(0, 0, 1));

	shader.Uniform("model", model);
}

void SpiritRenderer::SetCenterModelMatrix(glm::vec2 center, glm::vec2 size, float deg)
{
	shader.UseProgram();

	mat4 model(1.0f);
	model = translate(model, vec3(center, 0));
	model = rotate(model, (float)radians(deg), vec3(0, 0, 1));
	model = translate(model, vec3(-size.x / 2.0f, -size.y / 2.0f, 0));
	model = scale(model, vec3(size, 0));

	shader.Uniform("model", model);
}
