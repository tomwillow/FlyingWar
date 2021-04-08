#pragma once

#include "TTerra.h"

#include <unordered_map>

#include <glm/glm.hpp>

#include <FastNoise\FastNoise.h>

struct hash_ivec2
{
	size_t operator()(const glm::ivec2 &pt) const
	{
		return std::hash<int>{}(pt.x) + std::hash<int>{}(pt.x) ^ std::hash<int>{}(pt.y);
	}
};

class TerraManager
{
public:
	TerraManager(int seed = 0);
	~TerraManager();

	float GetAltitude(glm::vec3 pos);
	void Draw(glm::vec3 viewPos, glm::vec3 viewDir);

private:
	const float block_size = 10000.0f;
	const float step = 100.0f;
	const float max_altitude = 500.0f;
	std::vector<TTerra*> storage;
	std::unordered_map<glm::ivec2, TTerra*,hash_ivec2> terras;

	FastNoise noise;
	const float freq_coef = 0.1f;//freq值越大，噪点越密集
};