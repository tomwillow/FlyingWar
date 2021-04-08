
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

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

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
	plane(MODEL_PLAYER_PLANE),
	modelShader(SHADER_MODEL_VERTEX, SHADER_MODEL_FRAGMENT),
	terraShader(SHADER_TERRA_VERTEX, SHADER_TERRA_FRAGMENT),
	//ftVelocity("1000 km/h", FONT_CN)
	freetype(FONT_CN, 40),
	texArrow(TEXTURE_ARROW),
	texSpotPlane(TEXTURE_SPOT_PLANE),
	texSpotCamera(TEXTURE_SPOT_CAMERA),
	texSpot(TEXTURE_SPOT),
	modelBullet(MODEL_BULLET)
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
	camera->SetPosition(pos + vec3(0, 5, 15));
	camera->SetDirection(dir);
}

SceneLevel::~SceneLevel()
{
	for (auto bullet : bullets)
		delete bullet;
	bullets.clear();
}

void SceneLevel::BeforeGLClear(float dt)
{
	string info;
	if (falldown == false && pos.y <= terras.GetAltitude(pos))
	{
		controller->effectPlayer.Play(EFFECT_BOMB);
		falldown = true;
	}
	if (falldown)
		info = "飞机已坠毁。";

	if (player_is_dead)
	{
		controller->effectPlayer.Play(EFFECT_BOMB);
		info = "发生撞机。";
	}

	if (left_time <= 0)
	{
		left_time = 0;
		info = "回合结束。";
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (!info.empty())
	{
		pause = true;
		ImGui::OpenPopup(u8"排行榜");
	}

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(u8"排行榜", NULL, ImGuiWindowFlags_AlwaysAutoResize))//ImGuiWindowFlags_NoCollapse
	{
		//显示鼠标
		glfwSetInputMode(controller->glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		controller->bgmPlayer.Stop();
		controller->jetPlayer.Stop();

		ImGui::Text(string2utf8(info).c_str());

		bool highscore = controller->billboard.CanRefreshRecord(shot_down);//是否刷新记录
		static char buf[16] = u8"无名氏";
		if (highscore)
		{
			ImGui::Separator();
			ImGui::Text(string2utf8("新记录:"+to_string(shot_down)).c_str());
			ImGui::InputText("", buf, 16);
		}

		if (ImGui::Button(u8"确定"))
		{
			if (highscore)
			{
				controller->billboard.RefreshRecord(utf82string(buf), shot_down);
			}

			ImGui::CloseCurrentPopup();
			controller->GoCover();
		}
		ImGui::EndPopup();
	}

	// Rendering
	ImGui::Render();
}

void SceneLevel::Render(float dt)
{

	if (!pause)
	{
		UpdatePlayerPos(dt);
	}
	//设置摄像机
	{
		vec3 cpos = dir * dist;
		mat4 R_cpos(1.0f);
		R_cpos = rotate(R_cpos, radians(165.0f), right);
		cpos = vec3(R_cpos * vec4(cpos, 0));

		camera->SetPosition(pos + cpos);//vec3(0, 5, 15)
		camera->SetDirection(dir);
		camera->SetUp(up);
		//camera->SetPosition(pos + vec3(0, 5, 15));//
	}

	//
	UpdateAndDrawBullets(dt);
	UpdateAndDrawEnemies(dt);



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
		//camera->GetPosition(), camera->GetDirection();
		terras.Draw(camera->GetPosition(), camera->GetDirection());
	}



	//
	{
		mat4 Mlookat = lookAt(pos, pos + dir, up);

		modelShader.UseProgram();
		mat4 model(1.0f);

		model *= glm::inverse(Mlookat);

		//model = rotate(model, glm::pi<float>(), { 0,1,0 });
		modelShader.Uniform("model", model);
		modelShader.Uniform("view", camera->GetViewMatrix());
		modelShader.Uniform("projection", camera->GetProjection());
		modelShader.Uniform("viewPos", camera->GetPosition());

		vec3 lightPos1 = pos + vec3{ 0,10,5 };
		vec3 lightPos2 = pos + vec3{ 0,10,-5 };
		modelShader.Uniform("lightPos1", lightPos1);
		modelShader.Uniform("lightPos2", lightPos2);

		vec3 lightColor(0.5f);
		modelShader.Uniform("lightColor1", lightColor);
		modelShader.Uniform("lightColor2", lightColor);

		vec3 objectSpecular{ 0.5 };
		float objectShininess = 128;
		modelShader.Uniform("material.specular", objectSpecular);
		modelShader.Uniform("material.shininess", objectShininess);

		plane.Draw(modelShader);
		
#ifdef _DEBUG
		{
			//替代玩家的box
			model = mat4(1.0f);
			model = translate(model, pos);
			modelShader.Uniform("model", model);

			box.Bind();
			box.DrawTriangles();

		}
		{
			//dir axis
			mat4 Mlookat = lookAt(pos, pos + dir, up);

			model = mat4(1.0f);
			model *= glm::inverse(Mlookat);
			model = translate(model, vec3(0, 0, -1.5f));
			model = scale(model, vec3(0.1f, 0.1f, 3.0f));
			modelShader.Uniform("model", model);

			box.Bind();
			box.DrawTriangles();
		}
		{
			//up axis
			//model = mat4(1.0f);
			//model = translate(model, pos);
			//model = rotate(model, radians(real_pitch), right);
			//model = translate(model, vec3(0, 1.5f, 0));
			//model = scale(model, vec3(0.1f, 3.0f, 0.1f));
			//modelShader.Uniform("model", model);

			//box.Bind();
			//box.DrawTriangles();
		}
		{
			mat4 Mlookat = lookAt(pos, pos + right, up);

			//right axis
			model = mat4(1.0f);
			//model = rotate(model, -glm::pi<float>() / 2.0f, up);
			model *= glm::inverse(Mlookat);
			//model = translate(model, pos);
			//model = rotate(model, radians(-yaw), up);

			model = translate(model, vec3(1.5f, 0, 0));
			model = scale(model, vec3(3.0f, 0.1f, 0.1f));
			modelShader.Uniform("model", model);

			box.Bind();
			box.DrawTriangles();
		}
#endif
	}

	{
		//绘制天空盒
		skybox->Draw(camera->GetViewMatrix(), camera->GetProjection());

		camera->RefreshTime();
	}

	//绘制UI
	glDisable(GL_DEPTH_TEST);
	pureRenderer.DrawCenter(vec4(1, 1, 1, 0.5), vec2(W / 2.0f - 400.0f, H / 2.0f), vec2(180, 40));
	pureRenderer.DrawCenter(vec4(1, 1, 1, 0.5), vec2(W / 2.0f + 400.0f, H / 2.0f), vec2(180, 40));
	renderer.DrawCenter(texArrow, vec2(W / 2.0f - 400.0f + 200.0f, H / 2.0f), vec2(40, 40));
	renderer.DrawCenter(texArrow, vec2(W / 2.0f + 400.0f - 200.0f, H / 2.0f), vec2(-40, 40));
	renderer.DrawCenter(texSpotCamera, vec2(W / 2.0f, H / 2.0f), vec2(30, 20));
	renderer.DrawCenter(texSpotPlane, vec2(W / 2.0f, H / 2.0f), vec2(30, 20));

	freetype.DrawByPixel(renderer, to_string(int(v)) + " km/h", W / 2.0f - 400.0f - 80.0f, H / 2.0f - 15.0f);
	freetype.DrawByPixel(renderer, to_string(int(pos.y)) + " m", W / 2.0f + 400.0f - 80.0f, H / 2.0f - 15.0f);
	freetype.DrawByPixel(renderer, string("score: ") + to_string(shot_down), 80.0f, H - 120.0f);
	freetype.DrawByPixel(renderer, string("left time: ") + to_string((int)left_time), W-400.0f, H - 120.0f);

	//绘制敌机瞄准标志
	{
		for (auto enemy : enemies)
		{
			if (enemy->show)
			{
				vec2 screenCoord = enemy->GetScreenCoord({ W,H });
				renderer.DrawCenter(texSpot, screenCoord, vec2(40, 40), glfwGetTime() * 100.0f);
				//cout << "scr= " << screenCoord.x << " " << screenCoord.y << endl;

			}

		}
	}
	glEnable(GL_DEPTH_TEST);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//更新标题栏
	stringstream ss;
	ss << "pos=" << pos.x << " " << pos.y << " " << pos.z;
	ss << " dir=" << dir.x << " " << dir.y << " " << dir.z;
	//ss << " pitch=" << pitch << " real_pitch=" << real_pitch << " yaw=" << yaw;
	ss << " bullet num=" << bullets.size();
	ss << " enemies num=" << enemies.size();
	glfwSetWindowTitle(controller->glfw_window, ss.str().c_str());
}

void SceneLevel::AfterSwapBuffers(float dt)
{
}

void SceneLevel::OnKey(int key, int action)
{
#ifdef _DEBUG
	if (key == GLFW_KEY_0 && action == GLFW_RELEASE)
	{
		controller->GoCover();
	}
	if (key == GLFW_KEY_V && action == GLFW_RELEASE)
	{
		static bool hide_cursor = true;
		hide_cursor = !hide_cursor;
		if (hide_cursor)
		{
			glfwSetInputMode(controller->glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			pause = false;
		}
		else
		{
			glfwSetInputMode(controller->glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			pause = true;
		}
	}
#endif
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

void SceneLevel::UpdateAndDrawBullets(float dt)
{
	//计算子弹
	uniform_real_distribution<float> uni(0.02f, 0.15f);
	float bullet_interval = uni(eng);
	static float last_bullet = 0;
	float t = glfwGetTime();

	//按下空格，且流逝时间已超过冷却时间
	if (controller->keys[GLFW_KEY_SPACE] && t - last_bullet >= bullet_interval)
	{
		//新建子弹，速度增加200
		Bullet* bullet = new Bullet(pos, dir * v + (dir * 800.0f));
		bullets.push_back(bullet);

		controller->effectPlayer.Play(EFFECT_GUN);
	}

	for (auto it = bullets.begin(); it != bullets.end();)
	{
		auto& bullet = *it;
		if (!pause)
			bullet->UpdatePos(dt);
		bool dead = bullet->pos.y <= 0;//已坠地

		if (dead)
		{
			delete bullet;
			it = bullets.erase(it);
		}
		else
		{
			bullet->Draw(modelBullet,modelShader, *camera);
			it++;
		}
	}
}

void SceneLevel::UpdateAndDrawEnemies(float dt)
{
	const int enemy_limit = 20;
	if (enemies.size() < enemy_limit)
	{
		uniform_real_distribution<float> uni_angle(-45, 45);
		uniform_real_distribution<float> uni_r(200, 8000);
		uniform_real_distribution<float> uni_altitude(-100, 100);
		for (int i = enemies.size(); i < enemy_limit; ++i)
		{
			vec3 enemy_pos = dir * uni_r(eng);
			mat4 R(1.0f);
			R = translate(R, pos);
			R = rotate(R, uni_angle(eng), init_up);
			enemy_pos.y += uni_altitude(eng);
			if (enemy_pos.y < 200.0f)
				enemy_pos.y += 200.0f;
			enemy_pos = vec3(R * vec4(enemy_pos, 1));

			Enemy* enemy = new Enemy(enemy_pos, pos, 50.0f);
			enemies.push_back(enemy);

			//cout << "new enemy:" << enemy_pos.x << enemy_pos.y << enemy_pos.z << endl;
		}

	}


	for (auto it = enemies.begin(); it != enemies.end();)
	{
		auto& enemy = *it;
		if (!pause)
			enemy->UpdatePos(dt);

		float dist = distance(enemy->pos, pos);
		bool dead = dist > 1000;

		if (!dead)
		{
			for (auto bullet : bullets)
			{
				float dist = distance(bullet->pos, enemy->pos);
				//if (Conflict(*bullet, *enemy))
				if (dist < 20)
				{
					controller->effectPlayer.Play(EFFECT_BOMB);
					shot_down++;
					dead = true;
				}
			}
		}

		if (dead)
		{
			delete enemy;
			it = enemies.erase(it);
		}
		else
		{
			if (dist < 10.0f)
			{
				player_is_dead = true;
			}
			//if (it == enemies.begin())
			//{

			//	cout << "enemy:" << enemy->pos.x << enemy->pos.y << enemy->pos.z << endl;
			//	cout << "dist=" << dist << endl;
			//}
			enemy->Draw(plane, modelShader, *camera);
			it++;

		}
	}
}

void SceneLevel::UpdatePlayerPos(float dt)
{
	static float real_pitch = 0;
	//glm::vec3 front;
	//front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	//front.y = sin(glm::radians(pitch));
	//front.z = -cos(glm::radians(pitch)) * cos(glm::radians(yaw));

	//dir.x += cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	//dir.y += sin(glm::radians(pitch));

	real_pitch += pitch;

	mat4 R_front(1.0f);
	R_front = rotate(R_front, glm::radians(real_pitch), right);
	dir = vec3(R_front * vec4(init_dir, 0));

	{
		mat4 R(1.0f);
		R = rotate(R, radians(-yaw), up);
		dir = vec3(R * vec4(dir, 0));
	}

	up = vec3(R_front * vec4(init_up, 0));
	//dir.z += -cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	//dir = glm::normalize(front);

	{
		mat4 R(1.0f);
		R = rotate(R, radians(-yaw), dir);

		up = vec3(R * vec4(up, 0));

		right = vec3(R * vec4(init_right, 0));

	}

	if (pitch > 0)
		pitch -= dt * 1.0f;
	if (pitch < 0)
		pitch += dt * 1.0f;
	if (abs(pitch) < dt * 1.0f)
		pitch = 0;

	if (yaw > 0)
		yaw -= dt * 1.0f;
	if (yaw < 0)
		yaw += dt * 1.0f;

	//
	static float volume = 30;
	if (controller->keys[GLFW_KEY_W] && v < 2200.0f)
	{
		if (volume < 100.0f)
			volume += dt * 50.0f;
		else
			volume = 100.0f;
		controller->jetPlayer.SetVolume(volume);
		v += dt * 80.0f;
	}
	else
	{
		if (volume > 30.0f)
			volume -= dt * 80.0f;
		else
			volume = 30.0f;
		controller->jetPlayer.SetVolume(volume);
	}

	if (controller->keys[GLFW_KEY_S] && v > 180.0f)
	{
		v -= dt * 80.0f;
	}

	v -= dt * 1.0f;

	//
	float v_m_per_sec = v / 3.6f;
	pos += dir * (dt * v_m_per_sec);

	left_time -= dt;
}
