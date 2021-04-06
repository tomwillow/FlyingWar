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

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //禁用byte-alignment限制

	for (auto c : s)
	{
		if (c < 32)
			continue;
		if (dict.find(c) != dict.end())
			continue;

		FT_UInt glyph_index = FT_Get_Char_Index(face, c);
		if (glyph_index == 0)
		{
			//未定义字符会显示成方框
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

		//以灰度映射RGBA的所有通道
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

		/* 生成字形纹理 */
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// 设置纹理选项
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
		/* 纹理绑定完成 */

		// 将字符存储到字符表中备用
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


	// 对文本中的所有字符迭代
	for (auto c:s)
	{
		auto& ch = dict[c];
		GLfloat xpos = x + ch.Bearing.x;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y);

		GLfloat w = ch.Size.x;
		GLfloat h = ch.Size.y;

		// 在方块上绘制字形纹理
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		//
		glm::vec2 pos(xpos, ypos + h);
		renderer.Draw(ch.TextureID, pos, { w,-h });

		// 更新位置到下一个字形的原点，注意单位是1/64像素
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
