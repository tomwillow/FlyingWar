#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class TCamera
{
private:
	float screenWidth, screenHeight;
	float fov = 45.0;
	glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 15.0f);//��ʼλ��
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);//����ǰ������z������
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float diffTime = 0.0f;

	glm::mat4 view;
	glm::mat4 projection;
public:
	TCamera(float screenWidth, float screenHeight);

	//��Ⱦѭ���е���һ�μ���
	void RefreshTime();

	//ȡ��view����
	glm::mat4& GetViewMatrix();

	//ȡ��projection����
	glm::mat4& GetProjection();

	void SetPosition(glm::vec3 pos);
	void SetDirection(glm::vec3 dir);
	void SetUp(glm::vec3 up);

	glm::vec3& GetPosition();
	glm::vec3 GetTarget();
	glm::vec3& GetDirection();

	void SetPosDiff(glm::vec3 diff);

	//���ڴ�С�仯ʱ����
	void ProcessOnSize(float width,float height);

	//�����¼��е���
	void ProcessKeyBoard(GLFWwindow* window);

	//����¼��е���
	void ProcessMouseMovement(float xpos, float ypos);
};

