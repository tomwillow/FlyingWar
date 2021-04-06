#include <glad/glad.h>

#include <assert.h>

#include "TElementBuffer.h"

TElementBuffer::TElementBuffer(unsigned int bytes, const float* data, std::initializer_list<int> init_list, unsigned int indicesBytes, const unsigned int* indices) :TVertexArray(bytes, data, init_list)
{
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesBytes, indices, GL_STATIC_DRAW);

	indices_count = indicesBytes / sizeof(unsigned int);
}

TElementBuffer::~TElementBuffer()
{
	glDeleteBuffers(1, &EBO);
}

void TElementBuffer::Bind()
{
	TVertexArray::Bind();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}

//调用前需Bind，否则报错
void TElementBuffer::Draw(unsigned int mode)
{
#ifdef _DEBUG
	int currentVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
	assert(VAO == currentVAO);

	int currentEBO;
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentEBO);
	assert(EBO == currentEBO);
#endif

	glDrawElements(mode, indices_count, GL_UNSIGNED_INT, 0);
}