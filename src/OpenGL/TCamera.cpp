#include "TCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _DEBUG
#include <iostream>
#endif
using namespace glm;

using namespace std;

TCamera::TCamera(float screenWidth, float screenHeight):screenWidth(screenWidth),screenHeight(screenHeight)
{

}

void TCamera::RefreshTime()
{
	float nowTime = (float)glfwGetTime();
	static float lastTime = nowTime;
	diffTime = nowTime - lastTime;
	lastTime = nowTime;
}

glm::mat4& TCamera::GetViewMatrix()
{
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, worldUp);

	//quat q = quatLookAt(cameraFront, worldUp);
	//mat4 qm = mat4_cast(q);

	//view = mat4(1.0f);
	//view = translate(view, cameraPos);
	//view *= qm;

	return view;
}

glm::mat4& TCamera::GetProjection()
{
	projection = glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100000.0f);
	return projection;
}

void TCamera::SetPosition(glm::vec3 pos)
{
	cameraPos = pos;
}

void TCamera::SetDirection(glm::vec3 dir)
{
	cameraFront = dir;
}

void TCamera::SetUp(glm::vec3 up)
{
	worldUp = up;
}

glm::vec3& TCamera::GetPosition()
{
	return cameraPos;
}

glm::vec3 TCamera::GetTarget()
{
	return cameraPos+cameraFront;
}

glm::vec3& TCamera::GetDirection()
{
	return cameraFront;
}

void TCamera::SetPosDiff(glm::vec3 diff)
{
	cameraPos += diff;
}

void TCamera::ProcessOnSize(float width, float height)
{
	screenWidth = width;
	screenHeight = height;
}

void TCamera::ProcessKeyBoard(GLFWwindow *window)
{
	float cameraSpeed = diffTime * 5.0f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		glm::vec3 front_xz = cameraFront;
		front_xz.y = 0;
		cameraPos += cameraSpeed * glm::normalize(front_xz);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		glm::vec3 front_xz = cameraFront;
		front_xz.y = 0;
		cameraPos -= cameraSpeed * glm::normalize(front_xz);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glm::vec3 left = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraPos -= cameraSpeed * left;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		glm::vec3 left = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraPos += cameraSpeed * left;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		cameraPos.y -= cameraSpeed;
#ifdef _DEBUG
		//cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << endl;
#endif
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		cameraPos.y += cameraSpeed;
#ifdef _DEBUG
		//cout << cameraPos.x<<" "<<cameraPos.y<<" "<<cameraPos.z << endl;
#endif
	}

}

void TCamera::ProcessMouseMovement(float xpos,float ypos)
{

	static float lastX, lastY;
	static float yaw = 0.0f, pitch = 0.0f;
	static bool firstMouse = true;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;

	yaw += xoffset * sensitivity;
	pitch += yoffset * sensitivity;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));//
	front.y = sin(glm::radians(pitch));
	front.z = -cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	cameraFront = glm::normalize(front);
}