#include "Bullet.h"

using namespace glm;

Bullet::Bullet(glm::vec3 in_pos, glm::vec3 in_v):
	TObject(in_pos,in_v,r)
{
}

void Bullet::Draw(TShader& shader, TCamera& camera)
{

	shader.UseProgram();
	mat4 model(1.0f);
	model = translate(model, pos);
	shader.Uniform("model", model);
	shader.Uniform("view", camera.GetViewMatrix());
	shader.Uniform("projection", camera.GetProjection());
	shader.Uniform("viewPos", camera.GetPosition());

	vec3 lightPos1 = pos + vec3{ 0,10,5 };
	vec3 lightPos2 = pos + vec3{ 0,10,-5 };
	shader.Uniform("lightPos1", lightPos1);
	shader.Uniform("lightPos2", lightPos2);

	vec3 lightColor{ 1,0.9,0.9 };
	shader.Uniform("lightColor1", lightColor);
	shader.Uniform("lightColor2", lightColor);

	vec3 objectSpecular{ 0.5 };
	float objectShininess = 128;
	shader.Uniform("material.specular", objectSpecular);
	shader.Uniform("material.shininess", objectShininess);

	vertex.Bind();
	vertex.DrawTriangles();
}
