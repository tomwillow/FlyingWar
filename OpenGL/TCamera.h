#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class TCamera
{
private:
	float screenWidth, screenHeight;
	float fov = 45.0;
	glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 3.0f);//初始位置
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);//朝向前方，即z负方向
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float diffTime = 0.0f;

	glm::mat4 view;
	glm::mat4 projection;
public:
	TCamera(float screenWidth, float screenHeight);

	//渲染循环中调用一次即可
	void RefreshTime();

	//取得view矩阵
	glm::mat4& GetViewMatrix();

	//取得projection矩阵
	glm::mat4& GetProjection();

	glm::vec3& GetPosition();

	void SetPosDiff(glm::vec3 diff);

	//窗口大小变化时调用
	void ProcessOnSize(float width,float height);

	//键盘事件中调用
	void ProcessKeyBoard(GLFWwindow* window);

	//鼠标事件中调用
	void ProcessMouseMovement(float xpos, float ypos);
};

