#pragma once

#include "TObject.h"
#include "TBox.h"
#include "TShader.h"
#include "TCamera.h"
#include "Model.h"

class Bullet: public TObject
{
public:
	Bullet(glm::vec3 in_pos, glm::vec3 in_v);

	void Draw(Model & modelBullet,TShader& shader,TCamera &camera);
private:
	//TBox vertex;
	const float r = 1.0f;
};

