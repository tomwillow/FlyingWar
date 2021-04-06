#include "TFreeType.h"

#include <stdexcept>
#include <algorithm>

#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>

#pragma comment(lib,"freetype.lib")
#pragma comment(lib,"bzip2.lib")
#pragma comment(lib,"libpng16_static.lib")
#pragma comment(lib,"zlibstat.lib")

using namespace std;

void TFreeTypeDictionary::AddCharacter(std::tstring s)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		throw runtime_error("Could not init FreeType Library");

	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face))
		throw runtime_error("Failed to load font: " + font_name);

	if (FT_Set_Pixel_Sizes(face, 0, pixel_size))
		throw runtime_error("FT_Set_Pixel_Sizes error");

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //����byte-alignment����

	for (auto c : s)
	{
		if (c < 32)
			continue;
		if (dict.find(c) != dict.end())
			continue;

		FT_UInt glyph_index = FT_Get_Char_Index(face, c);
		if (glyph_index == 0)
		{
			//δ�����ַ�����ʾ�ɷ���
			throw no_char_error("FT_Get_Char_Index error: no this character");
		}
		if (FT_Load_Glyph(
			face, /* handle to face object */
			glyph_index, /* glyph index */
			FT_LOAD_COLOR)) /* load flags, see below */
			throw runtime_error("FT_Load_Glyph error");

		if (FT_Render_Glyph(face->glyph, /* glyph slot */
			FT_RENDER_MODE_NORMAL)) /* render mode */
			throw runtime_error("FT_Render_Glyph error");

		//�ԻҶ�ӳ��RGBA������ͨ��
		int pixels = face->glyph->bitmap.width * face->glyph->bitmap.rows;
		unsigned char* data = new unsigned char[4 * pixels];
		unsigned char* src = face->glyph->bitmap.buffer;
		for (int i = 0; i < pixels; ++i)
		{
			data[4 * i + 0] = src[i];
			data[4 * i + 1] = src[i];
			data[4 * i + 2] = src[i];
			data[4 * i + 3] = src[i];
		}

		/* ������������ */
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// ��������ѡ��
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,//GL_RED
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RGBA,//GL_RED
			GL_UNSIGNED_BYTE,
			data//face->glyph->bitmap.buffer
		);

		delete[] data;
		/* �������� */

		// ���ַ��洢���ַ����б���
		int w = face->glyph->bitmap.width;
		int h = face->glyph->bitmap.rows;

		Character ch = {
			texture,
			glm::ivec2(w, h),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		dict[c] = ch;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void TFreeTypeDictionary::DrawByPixel(SpiritRenderer& renderer, std::tstring s, GLfloat x, GLfloat y)
{
	for (auto c:s)
		if (dict.find(c) == dict.end())
		{
			AddCharacter(s);
			break;
		}


	// ���ı��е������ַ�����
	for (auto c:s)
	{
		auto& ch = dict[c];
		GLfloat xpos = x + ch.Bearing.x;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y);

		GLfloat w = ch.Size.x;
		GLfloat h = ch.Size.y;

		// �ڷ����ϻ�����������
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		//
		glm::vec2 pos(xpos, ypos + h);
		renderer.Draw(ch.TextureID, pos, { w,-h });

		// ����λ�õ���һ�����ε�ԭ�㣬ע�ⵥλ��1/64����
		x += (ch.Advance >> 6); //(2^6 = 64)
	}
}

TFreeTypeDictionary::TFreeTypeDictionary(std::string font_name, int pixel_size):
	font_name(font_name),pixel_size(pixel_size)
{
}

TFreeTypeDictionary::~TFreeTypeDictionary()
{
	for (auto& ch : dict)
	{
		glDeleteTextures(1, &ch.second.TextureID);
	}
}
