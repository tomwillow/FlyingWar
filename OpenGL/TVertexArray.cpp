#include "TVertexArray.h"


#include <glad/glad.h>

#include <assert.h>
#include <algorithm>
#include <numeric>

using namespace std;

TVertexArray::TVertexArray(unsigned int bytes, const float* data, std::initializer_list<int> init_list) :bytes(bytes)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
	
	SetAttribPointer(init_list);
}

TVertexArray::~TVertexArray()
{
	glDeleteVertexArrays(1, &VAO);
}


void TVertexArray::SetAttribPointer(std::initializer_list<int> init_list)
{
	//eg. {3,2} -> 5
	int num_in_a_group = accumulate(init_list.begin(), init_list.end(), 0);
	int index = 0;
	int start = 0;
	for (auto& n : init_list)
	{
		glVertexAttribPointer(index, n, GL_FLOAT, GL_FALSE, num_in_a_group * sizeof(float), (void*)(start*sizeof(float)));
		glEnableVertexAttribArray(index);
		index++;
		start += n;
	}

	group_count = bytes / sizeof(float) / num_in_a_group;

	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
}

void TVertexArray::Bind()
{
	glBindVertexArray(VAO);
}

void TVertexArray::Draw(unsigned int mode)
{
#ifdef _DEBUG
	int currentVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
	assert(VAO == currentVAO);
#endif

	glDrawArrays(mode, 0, group_count);
}

void TVertexArray::DrawTriangles()
{
	Draw(GL_TRIANGLES);
}

void TVertexArray::DrawLines()
{
	Draw(GL_LINES);
}

void TVertexArray::DrawQuads()
{
	Draw(GL_QUADS);
}
