#include "TBox.h"


float TBox::vertices[] = {
-0.5f, -0.5f, -0.5f,0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
 0.5f, -0.5f, -0.5f,0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
 0.5f,  0.5f, -0.5f,0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
 0.5f,  0.5f, -0.5f,0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
-0.5f,  0.5f, -0.5f,0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
-0.5f, -0.5f, -0.5f,0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

-0.5f, -0.5f,  0.5f,0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
 0.5f, -0.5f,  0.5f,0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
 0.5f,  0.5f,  0.5f,0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
 0.5f,  0.5f,  0.5f,0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
-0.5f,  0.5f,  0.5f,0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
-0.5f, -0.5f,  0.5f,0.0f,  0.0f, 1.0f,  0.0f, 0.0f,

-0.5f,  0.5f,  0.5f,-1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
-0.5f,  0.5f, -0.5f,-1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
-0.5f, -0.5f, -0.5f,-1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
-0.5f, -0.5f, -0.5f,-1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
-0.5f, -0.5f,  0.5f,-1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
-0.5f,  0.5f,  0.5f,-1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

 0.5f,  0.5f,  0.5f,1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
 0.5f,  0.5f, -0.5f,1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
 0.5f, -0.5f, -0.5f,1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
 0.5f, -0.5f, -0.5f,1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
 0.5f, -0.5f,  0.5f,1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
 0.5f,  0.5f,  0.5f,1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

-0.5f, -0.5f, -0.5f,0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
 0.5f, -0.5f, -0.5f,0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
 0.5f, -0.5f,  0.5f,0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
 0.5f, -0.5f,  0.5f,0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
-0.5f, -0.5f,  0.5f,0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
-0.5f, -0.5f, -0.5f,0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

-0.5f,  0.5f, -0.5f,0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
 0.5f,  0.5f, -0.5f,0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
 0.5f,  0.5f,  0.5f,0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
 0.5f,  0.5f,  0.5f,0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
-0.5f,  0.5f,  0.5f,0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
-0.5f,  0.5f, -0.5f,0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

TBox::TBox():TVertexArray(sizeof(vertices), vertices, { 3,3,2 })
{
}
