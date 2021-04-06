
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "SceneLevel.h"

#include "SceneController.h"
#include "ResMacro.h"
#include "ResValue.h"

//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <sstream>

#include <Windows.h>

#pragma comment(lib,"opengl32.lib")

#ifdef _DEBUG
#include <iostream>
#include <sstream>
#endif

using namespace std;
using namespace glm;

SceneLevel::SceneLevel(SceneController* in_controller) :
	Scene(in_controller),
	pureShader(SHADER_PURE_VERTEX, SHADER_PURE_FRAGMENT),
	pureRenderer(pureShader),
	plane("model/F35 Low Poly Model/F35 Low Poly Model.obj"),
	modelShader(SHADER_MODEL_VERTEX, SHADER_MODEL_FRAGMENT),
	terraShader(SHADER_TERRA_VERTEX, SHADER_TERRA_FRAGMENT),
	//ftVelocity("1000 km/h", FONT_CN)
	freetype(FONT_CN,40),
	texArrow(TEXTURE_ARROW),
	texSpotPlane(TEXTURE_SPOT_PLANE),
	texSpotCamera(TEXTURE_SPOT_CAMERA)
{
	camera = make_unique<TCamera>(W, H);
	skybox = make_unique<TSkyBox>(
		vector<string>{ TEXTURE_SKYBOX_RIGHT,
		TEXTURE_SKYBOX_LEFT,
		TEXTURE_SKYBOX_TOP,
		TEXTURE_SKYBOX_BOTTOM,
		TEXTURE_SKYBOX_FRONT,
		TEXTURE_SKYBOX_BACK },
		SHADER_SKYBOX_VERTEX,
		SHADER_SKYBOX_FRAGMENT
		);
	
	glfwSetInputMode(controller->glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	eng.seed(GetTickCount());

	//

	pureShader.UseProgram();
	mat4 view(1.0f);
	pureShader.Uniform("view", view);

	OnSize(W, H);

	//
	pos = born_pos;
	dir = init_dir;
	camera->SetPosition(pos+vec3(0,5,15));
	camera->SetDirection(dir);

}

void SceneLevel::Render(float dt)
{

	//glm::vec3 front;
	//front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	//front.y = sin(glm::radians(pitch));
	//front.z = -cos(glm::radians(pitch)) * cos(glm::radians(yaw));

	//dir.x += cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	//dir.y += sin(glm::radians(pitch));
	mat4 R_front(1.0f);
	R_front = rotate(R_front, glm::radians(pitch), right);
	dir = vec3(R_front * vec4(dir, 1));

	up = vec3(R_front * vec4(up, 1));
	//dir.z += -cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	//dir = glm::normalize(front);

	//{
	//	mat4 R(1.0f);
	//	R = rotate(R, radians(-yaw), up);

	//	dir = vec3(R * vec4(dir, 1));

	//	//right = vec3(R * vec4(right, 1));

	//}

	if (pitch>0)
		pitch -= dt * 1.0f;
	if (pitch<0)
		pitch += dt * 1.0f;

	if (yaw > 0)
		yaw -= dt * 1.0f;
	if (yaw < 0)
		yaw += dt * 1.0f;

	//
	if (controller->keys[GLFW_KEY_W] && v < 1200.0f)
	{
		v += dt * 50.0f;
	}
	if (controller->keys[GLFW_KEY_S] && v > 180.0f)
	{
		v -= dt * 50.0f;
	}

	v -= dt * 1.0f;

	camera->ProcessKeyBoard(controller->glfw_window);

	//camera->SetDirection(dir);

	//
	float v_m_per_sec = v / 3.6f;
	pos += dir * (dt * v_m_per_sec);

	{
		vec3 cpos = up * dist;
		mat4 R_cpos(1.0f);
		R_cpos = rotate(R_cpos, radians(angle), right);
		cpos = vec3(R_cpos * vec4(cpos, 1.0f));

		camera->SetPosition(pos + cpos);//vec3(0, 5, 15)
	}

	//
	{
		terraShader.UseProgram();
		mat4 model(1.0f);
		terraShader.Uniform("model", model);
		terraShader.Uniform("view", camera->GetViewMatrix());
		terraShader.Uniform("projection", camera->GetProjection());
		terraShader.Uniform("viewPos", camera->GetPosition());
		vec3 lightPos{ 0,10,0 };
		terraShader.Uniform("lightPos", lightPos);

		vec3 lightColor{ 1,1,1 };
		terraShader.Uniform("lightColor", lightColor);

		terras.Draw(camera->GetPosition(), camera->GetDirection());
	}

	//
	{
		modelShader.UseProgram();
		mat4 model(1.0f);
		model = translate(model, pos);
		model = rotate(model, glm::radians(yaw), { 0,0,-1 });
		model = rotate(model, glm::radians(-pitch), { 1,0,0 });
		model = rotate(model, glm::pi<float>(), { 0,1,0 });
		modelShader.Uniform("model", model);
		modelShader.Uniform("view", camera->GetViewMatrix());
		modelShader.Uniform("projection", camera->GetProjection());
		modelShader.Uniform("viewPos", camera->GetPosition());

		vec3 lightPos1=pos+vec3{ 0,10,5 };
		vec3 lightPos2=pos+vec3{ 0,10,-5 };
		modelShader.Uniform("lightPos1", lightPos1);
		modelShader.Uniform("lightPos2", lightPos2);

		vec3 lightColor{ 1,0.9,0.9 };
		modelShader.Uniform("lightColor1", lightColor);
		modelShader.Uniform("lightColor2", lightColor);

		vec3 objectSpecular{ 0.5 };
		float objectShininess = 128;
		modelShader.Uniform("material.specular", objectSpecular);
		modelShader.Uniform("material.shininess", objectShininess);

		plane.Draw(modelShader);

		{
			model = mat4(1.0f);
			model=translate(model, pos);
			modelShader.Uniform("model", model);

			box.Bind();
			box.DrawTriangles();

		}
	}

	{
		skybox->Draw(camera->GetViewMatrix(), camera->GetProjection());

		camera->RefreshTime();
	}

	glDisable(GL_DEPTH_TEST);
	pureRenderer.DrawCenter(vec4(1, 1, 1, 0.5), vec2(W / 2.0f - 400.0f, H / 2.0f), vec2(180, 40));
	pureRenderer.DrawCenter(vec4(1, 1, 1, 0.5), vec2(W / 2.0f + 400.0f, H / 2.0f), vec2(180, 40));
	renderer.DrawCenter(texArrow, vec2(W / 2.0f - 400.0f + 200.0f, H / 2.0f), vec2(40, 40));
	renderer.DrawCenter(texArrow, vec2(W / 2.0f + 400.0f - 200.0f, H / 2.0f), vec2(-40, 40));
	renderer.DrawCenter(texSpotCamera, vec2(W / 2.0f, H / 2.0f), vec2(30, 20));
	renderer.DrawCenter(texSpotPlane, vec2(W / 2.0f, H / 2.0f), vec2(30, 20));

	freetype.DrawByPixel(renderer, to_string(int(v)) + " km/h", W / 2.0f - 400.0f -80.0f, H / 2.0f -15.0f);
	freetype.DrawByPixel(renderer, to_string(int(pos.y)) + " m", W / 2.0f + 400.0f -80.0f, H / 2.0f -15.0f);
	glEnable(GL_DEPTH_TEST);

	stringstream ss;
	ss <<"pos="<< pos.x << " " << pos.y << " " << pos.z<<" pitch="<<pitch<<" yaw="<<yaw;
	glfwSetWindowTitle(controller->glfw_window, ss.str().c_str());
}

void SceneLevel::AfterSwapBuffers(float dt)
{
	if (pos.y <= terras.GetAltitude(pos))
	{
		AfterLose("飞机已坠毁。");
		return;
	}
}

void SceneLevel::OnKey(int key, int action)
{
	if (key == GLFW_KEY_0 && action == GLFW_RELEASE)
	{
		controller->GoCover();
	}
}

void SceneLevel::OnSize(int in_width, int in_height)
{
	this->Scene::OnSize(in_width, in_height);

	camera->ProcessOnSize(W, H);

	//
	pureShader.UseProgram();
	mat4 projection(1.0f);
	projection = ortho(0.0f, (float)W, 0.0f, (float)H);
	pureShader.Uniform("projection", projection);

}

void SceneLevel::OnMouseMove(double xpos, double ypos)
{
	//camera->ProcessMouseMovement(xpos, ypos);


	static float lastX, lastY;
	static bool firstMouse = true;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	//float xoffset = 0;
	float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.01f;

	yaw += xoffset * sensitivity;
	pitch += yoffset * sensitivity;

	const float pitch_limit = 45.0f;
	if (pitch > pitch_limit)
		pitch = pitch_limit;
	if (pitch < -pitch_limit)
		pitch = -pitch_limit;

	const float yaw_limit = 30.0f;
	if (yaw > yaw_limit)
		yaw = yaw_limit;
	if (yaw < -yaw_limit)
		yaw = -yaw_limit;

}

void SceneLevel::OnMouseButton(int button, int action)
{
}
void SceneLevel::AfterWin()
{
	HWND hWnd = glfwGetWin32Window(controller->glfw_window);
	if (MessageBox(hWnd, TEXT("Congratulations! You won!"), TEXT("Information"), MB_OK | MB_ICONINFORMATION) == IDOK)
	{
		controller->GoCover();
		return;
	}
}

void SceneLevel::AfterLose(std::string info)
{
	HWND hWnd = glfwGetWin32Window(controller->glfw_window);
	string text = info;
	if (MessageBox(hWnd, text.c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION) == IDOK)
	{
		controller->GoCover();
		return;
	}
}