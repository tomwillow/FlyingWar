#include "TPlane.h"

float TPlane::vertices[] = {
	-0.5f,0,-0.5f,0,1,0,0,0,//0
	-0.5f,0,0.5f,0,1,0,0,1,//1
	0.5f,0,0.5f,0,1,0,1,1,//2
	-0.5f,0,-0.5f,0,1,0,0,0,//0
	0.5f,0,0.5f,0,1,0,1,1,//2
	0.5f,0,-0.5f,0,1,0,1,0//3
};

TPlane::TPlane() :TVertexArray(sizeof(vertices), vertices, { 3,3,2 })
{
}
