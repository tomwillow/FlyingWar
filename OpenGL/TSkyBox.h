#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "TVertexArray.h"
#include "TShader.h"

class TSkyBox
{
public:
	//���� ��������ǰ�� ������պ���ͼ�����㡢Ƭ����ɫ���ļ���
	TSkyBox(std::vector<std::string> faces,std::string vsfile,std::string fsfile);
	~TSkyBox();

	//����view, projection���󣬻�����պ�
	//����˳��λ�����
	void Draw(glm::mat4 view, glm::mat4 projection);
private:
	TVertexArray vertex; 
	TShader skyboxShader; 
	unsigned int cubemapTexture;
};

