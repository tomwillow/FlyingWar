#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TObject
{
public:
	const glm::vec3 G = { 0,-9.8f,0 };
	glm::vec3 pos;
	glm::vec3 v;
	float r;

	TObject(glm::vec3 in_pos, glm::vec3 in_v, float in_r);

	virtual void UpdatePos(float dt);
};

bool Conflict(const TObject& a, const TObject& b);