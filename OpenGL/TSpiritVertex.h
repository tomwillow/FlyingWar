#pragma once

#include "TVertexArray.h"

class TSpiritVertex:public TVertexArray
{
private:
	//���ݸ�ʽ��x,y,texture
	static float vertices[];
public:
	TSpiritVertex();

	TSpiritVertex(const TSpiritVertex& other);
};

