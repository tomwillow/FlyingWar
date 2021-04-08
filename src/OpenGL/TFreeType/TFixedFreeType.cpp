#include "TFixedFreeType.h"

using namespace std;

TFixedFreeType::TFixedFreeType(std::tstring s, std::string font_name, float x_per, float y_per, float len_per, bool base_is_width, int pixel_size):
	TFixedSpirit(x_per,y_per,len_per,base_is_width),TFreeType(s,font_name,pixel_size)
{
	CombineToSingleBitmap();
}


void TFixedFreeType::CombineToSingleBitmap()
{
	int xmin = INT_MAX, xmax = INT_MIN;
	int ymin = INT_MAX, ymax = INT_MIN;

	xmin = min(xmin, Characters[0].Bearing.x);

	int x = 0, y = 0;

	// 对文本中的所有字符迭代
	for (auto& ch : Characters)
	{
		int xpos = x + ch.Bearing.x;
		int ypos = y - (ch.Size.y - ch.Bearing.y);

		int w = ch.Size.x;
		int h = ch.Size.y;

		xmax = max(xmax, xpos + w);
		ymin = min(ymin, ypos);
		ymax = max(ymax, ypos + h);

		// 更新位置到下一个字形的原点，注意单位是1/64像素
		x += ch.Advance >> 6; //(2^6 = 64)
	}
	width = xmax - xmin;
	height = ymax - ymin;
}

void TFixedFreeType::BindAndDraw_inner(float x1, float y1, float x2, float y2, int W, int H)
{
	float x = x1, y = y1;
	float xscale = (x2 - x1) / (float)width;
	float yscale = (y2 - y1) / (float)height;

	DrawByPixel(x, y, xscale, yscale);
}