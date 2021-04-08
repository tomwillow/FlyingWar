#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "TShader.h"
#include <fstream>
#include <sstream>

using namespace std;

int TShader::CompileShader(std::string fileName,unsigned int type)
{
	string shaderSource;
	ifstream ifs(fileName);
	if (ifs.fail())
		throw runtime_error("read file failed:" + fileName);
	ostringstream ss;
	ss << ifs.rdbuf();
	ifs.close();

	shaderSource = ss.str();

	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int shaderId = glCreateShader(type);
	const char* s = shaderSource.c_str();
	glShaderSource(shaderId, 1, &s, NULL);
	glCompileShader(shaderId);

	// check for shader compile errors
	int success;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
		throw runtime_error(fileName+infoLog);
	}

	return shaderId;
}

TShader::TShader(std::string vertexShaderFileName, std::string fragmentShaderFileName)
{
	int vertexShader=CompileShader(vertexShaderFileName, GL_VERTEX_SHADER);
	int fragmentShader=CompileShader(fragmentShaderFileName, GL_FRAGMENT_SHADER);

	// link shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// check for linking errors
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		throw runtime_error(string("ERROR::SHADER::PROGRAM::LINKING_FAILED") + infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

TShader::~TShader()
{
	glDeleteProgram(shaderProgram);
}

void TShader::UseProgram()const
{
	glUseProgram(shaderProgram);
#ifdef _DEBUG
	int err = glGetError();
	assert(err == GL_NO_ERROR);
#endif
}

int TShader::GetLocation(std::string name)const
{
#ifdef _DEBUG
	//未UseProgram就Uniform会报错
	int currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	assert(currentProgram = shaderProgram);
#endif

	int location = glGetUniformLocation(shaderProgram, name.c_str());

	//没有找到则报错
	assert(location >= 0);

	return location;
}

void TShader::Uniform(std::string name, float f0)const
{
	glUniform1f(GetLocation(name), f0);
}

void TShader::Uniform(std::string name, float f0, float f1, float f2)const
{
	glUniform3f(GetLocation(name), f0, f1, f2);
}

void TShader::Uniform(std::string name, float f0, float f1, float f2, float f3)const
{
	glUniform4f(GetLocation(name), f0, f1, f2, f3);
}

void TShader::Uniform(std::string name, int i0)const
{
	glUniform1i(GetLocation(name), i0);
}

void TShader::Uniform(std::string name, glm::vec3 v3)const
{
	Uniform(name, v3.x, v3.y, v3.z);
}

void TShader::Uniform(std::string name, glm::vec4 v4) const
{
	Uniform(name, v4.x, v4.y, v4.z, v4.w);
}

void TShader::Uniform(std::string name, glm::mat4& transform)const
{
	glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(transform));
}


void TShader::Uniform(std::string name, TTexture& texture, int samplerIndex)const
{
	texture.Bind(samplerIndex);
	Uniform(name, samplerIndex);
}

void TShader::Uniform(std::string name, unsigned int textureId, int samplerIndex)const
{
	glActiveTexture(GL_TEXTURE0 + samplerIndex); // 在绑定纹理之前先激活纹理单元
	glBindTexture(GL_TEXTURE_2D, textureId);
	Uniform(name, samplerIndex);
}
