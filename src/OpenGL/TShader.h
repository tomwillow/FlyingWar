#pragma once
#include <string>
#include <glm/glm.hpp>

#include "TTexture.h"

//�÷���
//����ǰuse, uniformǰҲ��Ҫuse

class TShader
{
private:
	unsigned int shaderProgram;
	int CompileShader(std::string fileName, unsigned int type);
	inline int GetLocation(std::string name)const;
public:
	TShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
	~TShader();

	void UseProgram()const;

	//����uniformǰ������UseProgram
	void Uniform(std::string name, float f0)const;
	void Uniform(std::string name, float f0, float f1, float f2)const;
	void Uniform(std::string name, float f0, float f1, float f2, float f3)const;
	void Uniform(std::string name, int i0)const;
	void Uniform(std::string name, glm::vec3 v3)  const;
	void Uniform(std::string name, glm::vec4 v4)  const;
	void Uniform(std::string name, glm::mat4 &transform)const;

	//������uniform
	void Uniform(std::string name, TTexture& texture, int samplerIndex=0)const;
	void Uniform(std::string name, unsigned int textureId, int samplerIndex)const;

};

