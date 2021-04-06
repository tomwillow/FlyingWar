#pragma once
#include "TVertexArray.h"
class TBox :
	public TVertexArray
{
private:
	//数据格式：x,y,z,normal,texture
	static float vertices[];
public:
	TBox();
};
