#pragma once

#include "TElementBuffer.h"

#include <FastNoise\FastNoise.h>

#include <tuple>

class TTerra :
	public TElementBuffer
{
private:
	//

	float width, height;
	float x_start, y_start;
	unsigned int point_count;
	unsigned int x_count, y_count;
	unsigned int x_point_count, y_point_count;
	unsigned int triangle_count;
	const int data_step = 6;
	float* terra;
	unsigned int* terra_indices;

	int xy2index(int x_index, int y_index);

	std::pair<int, int> index2xy(int point_index);

	int GetAltitude(int point_index);

	int GetAltitudeByPair(std::pair<int, int> pr);

	void CalcNormalVector(unsigned int x_index, unsigned int y_index,unsigned int point_index, unsigned int point_left_index, unsigned int point_right_index, unsigned int point_up_index, unsigned int point_down_index);

	void CalcNormalVector(unsigned int x_index, unsigned int y_index);

	//不用手动调用，构造函数中已经调用
	void Init(FastNoise& noise, float max_altitude, float step);
public:

	//长宽, x方向起始点, y方向起始点（实为z）, 网格步长
	TTerra(FastNoise& noise, float width, float height, float x_start, float y_start,float max_altitude, float step);
	virtual ~TTerra();

	void Erosion();

	void Update();

	void Release();

	static float GetAltitude(FastNoise& noise,float x, float y,float max_altitude);
};

