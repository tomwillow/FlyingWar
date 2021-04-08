#pragma once

#include "SpiritRenderer.h"

#include <Windows.h>
#include <gl\GL.h>

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <unordered_map>
#include <tstring.h>

#include <stdexcept>

struct Character {
	GLuint     TextureID;  // 字形纹理ID 由持有者负责销毁
	glm::ivec2 Size;       // 字形大小
	glm::ivec2 Bearing;    // 字形基于基线和起点的位置
	GLuint     Advance;    // 起点到下一个字形起点的距离，单位1/64像素
};

class TFreeTypeDictionary
{
public:	
	class no_char_error :public std::runtime_error
	{
	public:
		no_char_error(std::string s) :std::runtime_error(s) {}
	};

	TFreeTypeDictionary(std::string font_name, int pixel_size = 48);
	~TFreeTypeDictionary();

	//把字符串的所有字符加载进字典
	void AddCharacter(std::tstring s);

	//按照像素位置显示文字，x,y以左下角为原点
	//若s中的字符字典里没有，则会自动加载
	void DrawByPixel(SpiritRenderer& renderer,std::tstring s, GLfloat x, GLfloat y);

private:
	std::string font_name;
	int pixel_size;

	std::unordered_map<TCHAR, Character> dict;//字典
};

