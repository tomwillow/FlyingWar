#pragma once
#include "TVertexArray.h"
class TPlane :
    public TVertexArray
{
private:
	//数据格式：x,y,z,normal,texture
	static float vertices[];
public:
	TPlane();
};

