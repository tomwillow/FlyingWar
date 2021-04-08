#include "Enemy.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

Enemy::Enemy(glm::vec3 in_pos, glm::vec3 in_target, float v):
	TObject(in_pos,glm::normalize(in_target-in_pos)*v, 20.0f)
{
}

void Enemy::Draw(Model& plane, TShader& modelShader, TCamera& camera)
{
	vec3 up(0, 1, 0);
	vec3 dir = normalize(v);
	mat4 Mlookat = lookAt(pos, pos + dir, up);

	modelShader.UseProgram();
	mat4 model(1.0f);

	model *= glm::inverse(Mlookat);
	//model = translate(model, pos);
	//model = rotate(model, glm::radians(yaw), dir);
	//model = rotate(model, glm::radians(real_pitch), right);

	//model = rotate(model, glm::pi<float>(), { 0,1,0 });
	modelShader.Uniform("model", model);
	modelShader.Uniform("view", camera.GetViewMatrix());
	modelShader.Uniform("projection", camera.GetProjection());

	vec4 temp = camera.GetProjection() * camera.GetViewMatrix() * model * vec4(vec3(0),1.0f);
	show = temp.x > -temp.w && temp.x<temp.w && temp.y>-temp.w && temp.y<temp.w && temp.z>-temp.w && temp.z < temp.w;
	clipCoord = { temp.x / temp.w,temp.y / temp.w };

	modelShader.Uniform("viewPos", camera.GetPosition());

	vec3 lightPos1 = pos + vec3{ 0,10,5 };
	vec3 lightPos2 = pos + vec3{ 0,10,-5 };
	modelShader.Uniform("lightPos1", lightPos1);
	modelShader.Uniform("lightPos2", lightPos2);

	vec3 lightColor{ 1,0.9,0.9 };
	modelShader.Uniform("lightColor1", lightColor);
	modelShader.Uniform("lightColor2", lightColor);

	vec3 objectSpecular{ 0.5 };
	float objectShininess = 128;
	modelShader.Uniform("material.specular", objectSpecular);
	modelShader.Uniform("material.shininess", objectShininess);

	plane.Draw(modelShader);
}

glm::vec2 Enemy::GetScreenCoord(glm::vec2 screenSize)
{
	return { (clipCoord.x + 1.0f) / 2.0f * screenSize.x,(clipCoord.y + 1.0f) / 2.0f * screenSize.y };
}

void Enemy::UpdatePos(float dt)
{
	pos += v * dt;
}
