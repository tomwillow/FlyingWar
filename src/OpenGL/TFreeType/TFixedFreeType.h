#pragma once
#include "TFreeType.h"
#include "TFixedSpirit.h"

class TFixedFreeType :
    public TFreeType, public TFixedSpirit
{
protected:
	void CombineToSingleBitmap();
	virtual void BindAndDraw_inner(float x1, float y1, float x2, float y2, int W, int H) override;
public:
	TFixedFreeType(std::tstring s, std::string font_name, float x_per, float y_per, float len_per, bool base_is_width, int pixel_size=48);
};

