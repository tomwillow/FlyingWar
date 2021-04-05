#pragma once

#include "TShader.h"
#include "SpiritRenderer.h"

#include <glm/glm.hpp>

class PureRenderer:public SpiritRenderer
{
public:
	PureRenderer(TShader& in_shader);

	void Draw(glm::vec4 color, glm::vec2 position = glm::vec2(0, 0),
		glm::vec2 size = glm::vec2(10, 10), float deg = 0.0f);

	void DrawCenter(glm::vec4 color, glm::vec2 center = glm::vec2(0, 0),
		glm::vec2 size = glm::vec2(10, 10), float deg = 0.0f);
};

