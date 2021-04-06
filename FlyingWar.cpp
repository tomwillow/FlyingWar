//library
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _DEBUG
#pragma comment(lib,"glfw3d.lib")
#pragma comment(lib,"opengl32.lib")
#else
#pragma comment(lib,"glfw3.lib")
#endif

#include <vector>
#include <string>

//myself
#include "ResValue.h"
#include "ResMacro.h"
#include "SceneController.h"

using namespace std;
using namespace glm;

float W = DEFAULT_WIDTH, H = DEFAULT_HEIGHT;

unique_ptr<SceneController> controller;

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);



#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
#endif
{
	/* Initial */
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow* window = glfwCreateWindow(W, H, "Flying War", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, W, H);


	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);//注册鼠标动作
	glfwSetCursorPosCallback(window, mouse_callback);//注册鼠标动作
	glfwSetScrollCallback(window, scroll_callback);//注册滚轮动作
	glfwSetKeyCallback(window, key_callback);

	/* End of initialization */

	controller = make_unique<SceneController>(window);

	glEnable(GL_DEPTH_TEST);

	//面剔除
	//glEnable(GL_CULL_FACE);

	//帧混合
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(0.2f, 0.7f, 0.9f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		static float lastTime = glfwGetTime();
		float nowTime = glfwGetTime();
		float dt = nowTime - lastTime;

		processInput(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		controller->Render(dt);

		glfwSwapBuffers(window);
		glfwPollEvents();

		controller->AfterSwapBuffers(dt);

		lastTime = nowTime;
	}

	glfwTerminate();

	return 0;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	W = width;
	H = height;
	glViewport(0, 0, width, height);
	controller->OnSize(width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	controller->OnKey(key, action);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	controller->OnMouseButton(button, action);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	controller->OnMouseMove(xpos, H-ypos);
}

