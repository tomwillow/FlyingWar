#pragma once
#include "TVertexArray.h"
class TBox :
	public TVertexArray
{
private:
	//���ݸ�ʽ��x,y,z,normal,texture
	static float vertices[];
public:
	TBox();
};
