#include "TBoxFreeType.h"

#include <algorithm>

using namespace std;

TBoxFreeType::TBoxFreeType(std::tstring s, std::string font_name, int pixel_size) :
	TFreeType(s, font_name, pixel_size)
{
}

void TBoxFreeType::DrawByClipCoord(int w, int h, float x1, float y1, float x2, float y2)
{
	float x_size = (float)pixel_size / w * 2.0f, y_size = (float)pixel_size / h * 2.0f;

	float clip_xmin = xmin_pixel / (float)w * 2.0f;
	float clip_height = (float)height_pixel / (float)h * 2.0f;
	float clip_max_bearingY = (float)max_bearingY_pixel / (float)h * 2.0f;

	float x = x1 - clip_xmin*scale;
	float cy = y2 - clip_height / 2.0f*scale;
	float y = cy - (clip_max_bearingY - clip_height / 2.0f)*scale;

	float xscale = 2.0f / w, yscale = 2.0f / h;

	// 对文本中的所有字符迭代
	for (auto& ch : Characters)
	{
		if (ch.IsReturn || (x + x_size * scale > x2))
		{
			x = x1;
			y -= y_size * scale;
			if (ch.IsReturn)
			{
				continue;
			}
		}
		if (y < y1)
			break;

		GLfloat xpos = x + ch.Bearing.x * xscale*scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * yscale * scale;

		GLfloat w = ch.Size.x * xscale * scale;
		GLfloat h = ch.Size.y * yscale * scale;

		// 当前字符的VBO
		GLfloat vertices[4][2] = {
			{ xpos,     ypos + h },//,   0.0, 0.0
			{ xpos,     ypos},//,       0.0, 1.0 
			{ xpos + w, ypos},//,       1.0, 1.0 
			{ xpos + w, ypos + h}//,   1.0, 0.0 
		};
		// 在方块上绘制字形纹理
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);


		/** 开始绘制四边形 */
		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 0.0f); glVertex2f(vertices[0][0], vertices[0][1]);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(vertices[3][0], vertices[3][1]);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(vertices[2][0], vertices[2][1]);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(vertices[1][0], vertices[1][1]);
		glEnd();

		// 更新位置到下一个字形的原点，注意单位是1/64像素
		x += (ch.Advance >> 6) * xscale * scale; //(2^6 = 64)
	}
}

void TBoxFreeType::DrawVCenterByClipCoord(int w, int h, float x1, float y1, float x2, float y2)
{
	float cx = (x1 + x2) / 2.0f, cy = (y1 + y2) / 2.0f;

	float clip_xmin = (float)xmin_pixel / (float)w * 2.0f;
	float clip_width = (float)width_pixel / (float)w * 2.0f;
	float clip_height = (float)height_pixel / (float)h * 2.0f;
	float clip_max_bearingY = (float)max_bearingY_pixel / (float)h * 2.0f;

	//
	float x_size = (float)pixel_size / w * 2.0f, y_size = (float)pixel_size / h * 2.0f;

	float x = x1 - clip_xmin;
	float y = cy - (clip_max_bearingY - clip_height / 2.0f);

	float xscale = 2.0f / w, yscale = 2.0f / h;

	DrawByPixel(x, y, xscale, yscale);
}

float TBoxFreeType::GetXClipCoordInVCenter(int index, int w, int h, float x1, float y1, float x2, float y2)
{
	float cx = (x1 + x2) / 2.0f, cy = (y1 + y2) / 2.0f;

	float clip_xmin = (float)xmin_pixel / (float)w * 2.0f;
	float clip_height = (float)height_pixel / (float)h * 2.0f;
	float clip_max_bearingY = (float)max_bearingY_pixel / (float)h * 2.0f;

	//
	float x_size = (float)pixel_size / w * 2.0f, y_size = (float)pixel_size / h * 2.0f;

	float x = x1 - clip_xmin;
	float y = cy - (clip_max_bearingY - clip_height / 2.0f);

	float xscale = 2.0f / w, yscale = 2.0f / h;

	// 对文本中的所有字符迭代
	for (int i=0;i<index;++i)
	{
		auto& ch = Characters[i];
		GLfloat xpos = x + ch.Bearing.x * xscale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * yscale;

		GLfloat w = ch.Size.x * xscale;
		GLfloat h = ch.Size.y * yscale;

		// 更新位置到下一个字形的原点，注意单位是1/64像素
		x += (ch.Advance >> 6) * xscale; //(2^6 = 64)
	}
	return x;
}

void TBoxFreeType::DrawCenterByClipCoord(int w, int h, float x1, float y1, float x2, float y2)
{
	float cx = (x1 + x2) / 2.0f, cy = (y1 + y2) / 2.0f;


	float clip_xmin = (float)xmin_pixel / (float)w * 2.0f*scale;
	float clip_width = (float)width_pixel / (float)w * 2.0f * scale;
	float clip_height = (float)height_pixel / (float)h * 2.0f * scale;
	float clip_max_bearingY = (float)max_bearingY_pixel / (float)h * 2.0f * scale;

	//
	//float x_size = (float)pixel_size / w * 2.0f, y_size = (float)pixel_size / h * 2.0f;

	float x = cx - (clip_xmin + clip_width / 2.0f);
	float y = cy - (clip_max_bearingY - clip_height / 2.0f);

	float xscale = 2.0f / w, yscale = 2.0f / h;

	DrawByPixel(x, y, xscale, yscale);
}
