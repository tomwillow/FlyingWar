#include "TerraManager.h"

TerraManager::TerraManager(int seed)
{
	noise.SetFrequency(freq_coef * 1.0f / max_altitude);//freq值越大，噪点越密集
	noise.SetInterp(FastNoise::Quintic);
}

TerraManager::~TerraManager()
{
	for (auto p : storage)
		delete p;
}

float TerraManager::GetAltitude(glm::vec3 pos)
{
	float x = pos.x;
	float z = pos.z;
	return max_altitude * noise.GetPerlin(x, z);
}

void TerraManager::Draw(glm::vec3 viewPos, glm::vec3 viewDir)
{
	float x = viewPos.x;
	float z = viewPos.z;
	int ix = x / block_size;
	int iz = z / block_size;
	const int range = 3;
	for (int i=ix-range;i<=ix+range;++i)
		for (int j = iz - range; j < iz + range; ++j)
		{
			auto it = terras.find({ i,j });
			if (it == terras.end())
			{
				TTerra* new_block = new TTerra(noise,block_size, block_size, (float)i * block_size, (float)j * block_size, max_altitude, step);
				new_block->Update();
				storage.push_back(new_block);

				terras[{i, j}] = new_block;
			}
			terras[{i, j}]->Bind();
			terras[{i, j}]->DrawTriangles();
		}
}