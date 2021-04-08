#pragma once

#include "TElementBuffer.h"

#include <FastNoise\FastNoise.h>

#include <tuple>

class TTerra :
	public TElementBuffer
{
private:
	//

	float width, height;//宽 高
	float x_start, y_start;//x y起始点
	unsigned int point_count;//总点数=x_point_count*y_point_count
	unsigned int x_count, y_count;//x y区段数量 = x y方向点数量+1
	unsigned int x_point_count, y_point_count;//x y方向点数量
	unsigned int triangle_count;//三角形数量
	const int data_step = 8;//每组数据含 [3个点坐标，3个法向量坐标，2个贴图坐标] = 6个
	float* terra;//点集数据
	unsigned int* terra_indices;//索引数据

	int xy2index(int x_index, int y_index);

	std::pair<int, int> index2xy(int point_index);

	int GetAltitude(int point_index);

	int GetAltitudeByPair(std::pair<int, int> pr);

	void CalcNormalVector(unsigned int x_index, unsigned int y_index,unsigned int point_index, unsigned int point_left_index, unsigned int point_right_index, unsigned int point_up_index, unsigned int point_down_index);

	void CalcNormalVector(unsigned int x_index, unsigned int y_index);

	//不用手动调用，构造函数中已经调用
	void Init(FastNoise& noise, float max_altitude, float step);
public:

	//长，宽, x方向起始点, y方向起始点（实为z），最大海拔，网格步长
	TTerra(FastNoise& noise, float width, float height, float x_start, float y_start,float max_altitude, float step);
	virtual ~TTerra();

	void Erosion();

	void Update();

	void Release();

	static float GetAltitude(FastNoise& noise,float x, float y,float max_altitude);
};

