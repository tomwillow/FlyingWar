#pragma once

#include "TShader.h"
#include "TTexture.h"
#include "TSpiritVertex.h"

#include <glm/glm.hpp>

class SpiritRenderer
{
public:
	SpiritRenderer(TShader &in_shader);

	void Draw(TTexture& tex, glm::vec2 position = glm::vec2(0, 0),
		glm::vec2 size = glm::vec2(10, 10), float deg = 0.0f);

	void Draw(int tex_id, glm::vec2 position = glm::vec2(0, 0),
		glm::vec2 size = glm::vec2(10, 10), float deg = 0.0f);

	void DrawCenter(TTexture& tex, glm::vec2 position = glm::vec2(0, 0),
		glm::vec2 size = glm::vec2(10, 10), float deg = 0.0f);
protected:
	TShader &shader;
	TSpiritVertex spirit;
	void SetModelMatrix(glm::vec2 position, glm::vec2 size, float deg);
	void SetCenterModelMatrix(glm::vec2 center, glm::vec2 size, float deg);
private:
};

