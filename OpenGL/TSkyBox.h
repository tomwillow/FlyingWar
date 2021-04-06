#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "TVertexArray.h"
#include "TShader.h"

class TSkyBox
{
public:
	//按照 右左上下前后 传入天空盒贴图，顶点、片段着色器文件名
	TSkyBox(std::vector<std::string> faces,std::string vsfile,std::string fsfile);

	//传入view, projection矩阵，绘制天空盒
	//绘制顺序位于最后
	void Draw(glm::mat4 view, glm::mat4 projection);
private:
	TVertexArray vertex; 
	TShader skyboxShader; 
	unsigned int cubemapTexture;
};

