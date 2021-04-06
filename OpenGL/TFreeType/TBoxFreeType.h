#pragma once
#include "TFreeType.h"
class TBoxFreeType :
    public TFreeType
{
public:
    TBoxFreeType(std::tstring s, std::string font_name, int pixel_size = 48);

    void DrawByClipCoord(int w, int h, float x1, float y1, float x2, float y2);
    void DrawVCenterByClipCoord(int w, int h, float x1, float y1, float x2, float y2);
    float GetXClipCoordInVCenter(int index, int w, int h, float x1, float y1, float x2, float y2);

    void DrawCenterByClipCoord(int w, int h, float x1, float y1, float x2, float y2);

};

