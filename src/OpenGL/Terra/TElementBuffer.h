#pragma once
#include "TVertexArray.h"
class TElementBuffer :
	public TVertexArray
{
protected:
	TElementBuffer() {}
private:
	unsigned int EBO;
	unsigned int indices_count;
	virtual void Draw(unsigned int mode) override;
public:
	TElementBuffer(unsigned int bytes, const float* data, std::initializer_list<int> init_list, unsigned int indicesBytes, const unsigned int* indices);
	virtual ~TElementBuffer();

	virtual void Bind() override;
};

