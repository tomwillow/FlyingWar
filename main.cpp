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
#include <stb_image.h>
#include <TVertexArray.h>

//myself
#include "ResValue.h"
#include "SceneController.h"
#include "TCamera.h"


using namespace std;
using namespace glm;

float W = DEFAULT_WIDTH, H = DEFAULT_HEIGHT;

//unique_ptr<SceneController> controller;
TCamera camera(W,H);

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

GLuint loadCubemap(std::vector<std::string> in_filename)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < in_filename.size(); i++)
	{
		unsigned char* data = stbi_load(in_filename[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			int format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			throw std::runtime_error("Failed to load texture:" + in_filename[i]);
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

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
	GLFWwindow* window = glfwCreateWindow(W, H, "Simple Racer", NULL, NULL);
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

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);//注册鼠标动作
	glfwSetCursorPosCallback(window, mouse_callback);//注册鼠标动作
	glfwSetScrollCallback(window, scroll_callback);//注册滚轮动作
	glfwSetKeyCallback(window, key_callback);

	/* End of initialization */

	//controller = make_unique<SceneController>(window);

	glEnable(GL_DEPTH_TEST);

	//面剔除
	glEnable(GL_CULL_FACE);

	//帧混合
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vector<string> faces;
	faces.push_back("textures/skybox/right.jpg");
	faces.push_back("textures/skybox/left.jpg");
	faces.push_back("textures/skybox/top.jpg");
	faces.push_back("textures/skybox/bottom.jpg");
	faces.push_back("textures/skybox/front.jpg");
	faces.push_back("textures/skybox/back.jpg");
	GLuint cubemapTexture = loadCubemap(faces);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	TVertexArray skybox(sizeof(skyboxVertices), skyboxVertices, { 3 });


	TShader skyboxShader("shader/skybox.vs", "shader/skybox.fs");

	skyboxShader.UseProgram();
	skyboxShader.Uniform("skybox", 0);

	glClearColor(0.2f, 0.7f, 0.9f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		static float lastTime = glfwGetTime();
		float nowTime = glfwGetTime();
		float dt = nowTime - lastTime;

		processInput(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.UseProgram();
		glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.Uniform("view", view);
		skyboxShader.Uniform("projection", camera.GetProjection());
		// skybox cube
		skybox.Bind();
		//glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		skybox.DrawTriangles();
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		//glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		//controller->Render(dt);

		glfwSwapBuffers(window);
		glfwPollEvents();

		//controller->AfterSwapBuffers(dt);

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
	//camera.ProcessOnSize(W, H);
	//controller->OnSize(width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//controller->OnKey(key, action);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//camera.ProcessKeyBoard(window);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//controller->OnMouseButton(button, action);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//controller->OnMouseMove(xpos, H-ypos);
	camera.ProcessMouseMovement(xpos, ypos);
}

