#pragma once
#include <GLFW/glfw3.h>
#include <string>

class TTexture
{
private:
	unsigned int textureId;
	std::string filename;
public:

	//jpg: channels=3; png: channels=4;
	TTexture(std::string in_filename,unsigned int filtering=GL_LINEAR);
	TTexture(const TTexture& other);

	~TTexture();

	void Bind(int samplerIndex=0);
};

