#pragma once
#include "TVertexArray.h"
class TPlane :
    public TVertexArray
{
private:
	//���ݸ�ʽ��x,y,z,normal,texture
	static float vertices[];
public:
	TPlane();
};

