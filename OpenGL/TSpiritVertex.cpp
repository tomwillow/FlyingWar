#include "TSpiritVertex.h"

float TSpiritVertex::vertices[] = {
	0,0,0,0,//0
	1,0,1,0,//2
	0,1,0,1,//1
	0,1,0,1,//1
	1,0,1,0,//2
	1,1,1,1//3
};

TSpiritVertex::TSpiritVertex() :TVertexArray(sizeof(vertices), vertices, { 2,2 })
{
}

TSpiritVertex::TSpiritVertex(const TSpiritVertex& other) : TVertexArray(sizeof(vertices), vertices, { 2,2 })
{
}
