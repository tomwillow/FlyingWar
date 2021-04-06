#include "TTerra.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <iostream>
#include <algorithm>
#include <random>
#include <assert.h>
#include <glm/glm.hpp>

using namespace std;

int TTerra::xy2index(int x_index, int y_index)
{
	return y_index * y_point_count + x_index;
}

pair<int,int> TTerra::index2xy(int point_index)
{
	return { point_index / y_point_count,point_index % y_point_count };
}

int TTerra::GetAltitude(int point_index)
{
	return point_index * data_step + 1;
}

int TTerra::GetAltitudeByPair(pair<int, int> pr)
{
	return xy2index(pr.first, pr.second) * data_step + 1;
}

void TTerra::CalcNormalVector(unsigned int x_index, unsigned int y_index,unsigned int point_index,unsigned int point_left_index,unsigned int point_right_index,unsigned int point_up_index,unsigned int point_down_index)
{

	//根据始末点得到方向向量
	auto GetAxleByPointIndex = [&](unsigned int pt_start_index, unsigned int pt_end_index)->glm::vec3
	{
		glm::vec3 axle;
		axle.x = terra[pt_end_index * data_step] - terra[pt_start_index * data_step];
		axle.y = terra[pt_end_index * data_step + 1] - terra[pt_start_index * data_step + 1];
		axle.z = terra[pt_end_index * data_step + 2] - terra[pt_start_index * data_step + 2];
		return axle;
	};

	//按照右手定则对每个点取两个方向向量，以计算法向量
	//在平面情况下，法向量为y正方向
	glm::vec3 norm, axle1, axle2;
	if (y_index == y_count && x_index == x_count)
	{
		axle1 = GetAxleByPointIndex(point_index, point_left_index);
		axle2 = GetAxleByPointIndex(point_index, point_down_index);
	}
	else
	{
		if (y_index == y_count)
		{
			axle1 = GetAxleByPointIndex(point_index, point_down_index);
			axle2 = GetAxleByPointIndex(point_index, point_right_index);
		}
		else
		{
			if (x_index == x_count)
			{
				axle1 = GetAxleByPointIndex(point_index, point_up_index);
				axle2 = GetAxleByPointIndex(point_index, point_left_index);
			}
			else
			{
				axle1 = GetAxleByPointIndex(point_index, point_right_index);
				axle2 = GetAxleByPointIndex(point_index, point_up_index);
			}
		}
	}

	norm = -glm::normalize(glm::cross(axle1, axle2));//glm的叉乘似乎是左手定则，所以加负号

	//写入法向量
	terra[point_index * data_step + 3] = norm.x;
	terra[point_index * data_step + 4] = norm.y;
	terra[point_index * data_step + 5] = norm.z;

}

void TTerra::CalcNormalVector(unsigned int x_index, unsigned int y_index)
{
	//计算法向量
	unsigned int point_down_index = (y_index - 1) * y_point_count + x_index;
	unsigned int point_left_index = y_index * y_point_count + x_index - 1;

	unsigned int point_index = y_index * y_point_count + x_index;
	unsigned int point_right_index = y_index * y_point_count + x_index + 1;
	unsigned int point_right_up_index = (y_index + 1) * y_point_count + x_index + 1;
	unsigned int point_up_index = (y_index + 1) * y_point_count + x_index;

	CalcNormalVector(x_index, y_index, point_index, point_left_index, point_right_index, point_up_index, point_down_index);
}

void TTerra::Init(FastNoise &noise,float max_altitude, float step)
{
	x_count = width / step;//x方向段数
	y_count = height / step;//y方向段数
	x_point_count = x_count + 1;//x方向点数
	y_point_count = y_count + 1;//y方向点数
	assert(x_point_count >= 2 && y_point_count >= 2);

	//点集数据
	point_count = x_point_count * y_point_count;
	terra = new float[point_count * data_step];

	//索引数据
	triangle_count = x_count * y_count * 2;//三角形数量为方块数量2倍
	terra_indices = new unsigned int[triangle_count * 3];//索引点数量为三角形数量3倍

	//noise.SetSeed(0);

	//FastNoise noise2;
	//noise2.SetFrequency(0.5f * 1.0f / altitude);
	//noise2.SetInterp(FastNoise::Quintic);

	//FastNoise noise3;
	//noise3.SetFrequency(1.0f * 1.0f / altitude);
	//noise3.SetInterp(FastNoise::Quintic);

	//FastNoise noise4;
	//noise4.SetFrequency(8.0f * 1.0f / altitude);
	//noise4.SetInterp(FastNoise::Quintic);

	{
		int pt_index = 0;
		float y = y_start;
		for (int i = 0; i < y_point_count; ++i)
		{
			float x = x_start;
			for (int j = 0; j < x_point_count; ++j)
			{
				terra[pt_index * data_step] = x;// + noise2.GetPerlin(x, y) * 0.5f + noise3.GetPerlin(x, y) * 0.05f + noise4.GetPerlin(x, y) * 0.005f
				terra[pt_index * data_step + 1] = max_altitude *noise.GetPerlin(x, y);//  0.1*uni(e);//随机高度
				terra[pt_index * data_step + 2] = y;

				x += step;
				pt_index++;
			}

			y += step;
		}
	}

	{
		int index = 0;
		for (int y_index = 0; y_index < y_count + 1; ++y_index)//循环遍历所有点
		{
			for (int x_index = 0; x_index < x_count + 1; ++x_index)
			{
				unsigned int point_index = y_index * y_point_count + x_index;
				unsigned int point_right_index = y_index * y_point_count + x_index + 1;
				unsigned int point_right_up_index = (y_index + 1) * y_point_count + x_index + 1;
				unsigned int point_up_index = (y_index + 1) * y_point_count + x_index;

				if (y_index < y_count && x_index < x_count)//索引三角形总数=x_count*y_count*2
				{
					//下三角 逆时针
					terra_indices[index * 3] = point_index;
					terra_indices[index * 3 + 1] = point_right_index;
					terra_indices[index * 3 + 2] = point_right_up_index;
					index++;

					//上三角 逆时针
					terra_indices[index * 3] = point_index;
					terra_indices[index * 3 + 1] = point_right_up_index;
					terra_indices[index * 3 + 2] = point_up_index;
					index++;
				}


				//计算法向量
				unsigned int point_down_index = (y_index - 1) * y_point_count + x_index;
				unsigned int point_left_index = y_index * y_point_count + x_index - 1;

				CalcNormalVector(x_index, y_index, point_index, point_left_index, point_right_index, point_up_index, point_down_index);
			}
		}
	}
}

void TTerra::Erosion()
{
	auto FindNextPoint = [=](unsigned int x_index,unsigned int y_index, int last_x_index, int last_y_index)->pair<int, int>
	{
		pair<int, int> ret{ -1,-1 };
		int origin_index = xy2index(x_index, y_index);
		float h = terra[GetAltitude(origin_index)];
		float dest_h = h;
		for (int i = max(y_index - 1, 0u); i <= min(y_index + 1, y_point_count - 1); ++i)// && i!=y_index && i != last_y_index
			for (int j = max(x_index - 1, 0u); j <= min(x_index + 1, x_point_count - 1); ++j)// && j!=x_index && j != last_x_index
			{
				int index = xy2index(j, i);
				if (terra[GetAltitude(index)] < dest_h)
				{
					ret = { j,i };
					dest_h = terra[GetAltitude(index)];
				}
			}
		return ret;
	};

	int droplet_count = 1000;
	default_random_engine e;
	e.seed(glfwGetTime()*100.0f);
	uniform_int_distribution<int> x_uni(0, x_count);
	uniform_int_distribution<int> y_uni(0, y_count);
	while (droplet_count--)
	{
		int x_index = x_uni(e);
		int y_index = y_uni(e);
		int last_x_index = x_index;
		int last_y_index = y_index;
		pair<int, int> last{ x_index,y_index };

		int life = 300;
		float soil = 0.0f;
		while (life--)
		{
			auto pr = FindNextPoint(x_index, y_index, last_x_index, last_y_index);
			if (pr == pair<int, int>{-1, -1})
			{
				terra[GetAltitudeByPair(last)] += soil;
				break;
			}

			float dh = terra[GetAltitudeByPair(last)] - terra[GetAltitudeByPair(pr)];

			float lost = dh * 0.005f;
			terra[GetAltitudeByPair(last)] -= lost;
			CalcNormalVector(last.first,last.second);

			//cout << lost << endl;

			last_x_index = last.first;
			last_y_index = last.second;

			x_index = pr.first;
			y_index = pr.second;

			last = pr;
			soil += lost;
		}

	}
}

void TTerra::Update()
{
	//this->TVertexArray::Bind();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//GLint size = 0;
	//glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	glBufferSubData(GL_ARRAY_BUFFER, 0, point_count * data_step * sizeof(float), terra);//

#ifdef _DEBUG
	assert(glGetError() == 0);
#endif
}

void TTerra::Release()
{
	delete[] terra;
	delete[] terra_indices;
}

float TTerra::GetAltitude(FastNoise& noise, float x, float y, float max_altitude)
{
	return max_altitude * noise.GetPerlin(x, y);
}

TTerra::TTerra(FastNoise& noise, float width,float height,float x_start,float y_start, float max_altitude, float step):width(width),height(height),x_start(x_start),y_start(y_start)
{
	Init(noise,max_altitude,step);

	//erosion
	//Erosion();

	//生成地形EBO
	this->TElementBuffer::TElementBuffer(point_count * data_step * sizeof(float), terra, { 3,3 }, triangle_count * 3 * sizeof(unsigned int), terra_indices);
}

TTerra::~TTerra()
{
	Release();
}