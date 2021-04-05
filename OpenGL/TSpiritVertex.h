#pragma once

#include "TVertexArray.h"

class TSpiritVertex:public TVertexArray
{
private:
	//数据格式：x,y,texture
	static float vertices[];
public:
	TSpiritVertex();

	TSpiritVertex(const TSpiritVertex& other);
};

