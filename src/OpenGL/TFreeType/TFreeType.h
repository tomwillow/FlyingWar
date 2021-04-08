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
	GLuint     TextureID;  // ��������ID �ɳ����߸�������
	glm::ivec2 Size;       // ���δ�С
	glm::ivec2 Bearing;    // ���λ��ڻ��ߺ�����λ��
	GLuint     Advance;    // ��㵽��һ���������ľ��룬��λ1/64����
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

	//���ַ����������ַ����ؽ��ֵ�
	void AddCharacter(std::tstring s);

	//��������λ����ʾ���֣�x,y�����½�Ϊԭ��
	//��s�е��ַ��ֵ���û�У�����Զ�����
	void DrawByPixel(SpiritRenderer& renderer,std::tstring s, GLfloat x, GLfloat y);

private:
	std::string font_name;
	int pixel_size;

	std::unordered_map<TCHAR, Character> dict;//�ֵ�
};

