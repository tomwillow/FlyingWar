#pragma once

#include "TElementBuffer.h"

#include <FastNoise\FastNoise.h>

#include <tuple>

class TTerra :
	public TElementBuffer
{
private:
	//

	float width, height;//�� ��
	float x_start, y_start;//x y��ʼ��
	unsigned int point_count;//�ܵ���=x_point_count*y_point_count
	unsigned int x_count, y_count;//x y�������� = x y���������+1
	unsigned int x_point_count, y_point_count;//x y���������
	unsigned int triangle_count;//����������
	const int data_step = 8;//ÿ�����ݺ� [3�������꣬3�����������꣬2����ͼ����] = 6��
	float* terra;//�㼯����
	unsigned int* terra_indices;//��������

	int xy2index(int x_index, int y_index);

	std::pair<int, int> index2xy(int point_index);

	int GetAltitude(int point_index);

	int GetAltitudeByPair(std::pair<int, int> pr);

	void CalcNormalVector(unsigned int x_index, unsigned int y_index,unsigned int point_index, unsigned int point_left_index, unsigned int point_right_index, unsigned int point_up_index, unsigned int point_down_index);

	void CalcNormalVector(unsigned int x_index, unsigned int y_index);

	//�����ֶ����ã����캯�����Ѿ�����
	void Init(FastNoise& noise, float max_altitude, float step);
public:

	//������, x������ʼ��, y������ʼ�㣨ʵΪz������󺣰Σ����񲽳�
	TTerra(FastNoise& noise, float width, float height, float x_start, float y_start,float max_altitude, float step);
	virtual ~TTerra();

	void Erosion();

	void Update();

	void Release();

	static float GetAltitude(FastNoise& noise,float x, float y,float max_altitude);
};

