
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "SceneLevel.h"

#include "SceneController.h"
#include "ResMacro.h"
#include "ResValue.h"

//
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
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
	modelBullet(MODEL_BULLET),
	texGround(TEXTURE_GROUND)
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

}

SceneLevel::~SceneLevel()
{
	for (auto bullet : bullets)
		delete bullet;
	bullets.clear();

	for (auto enemy : enemies)
		delete enemy;
	enemies.clear();
}

void SceneLevel::BeforeGLClear(float dt)
{

	if (left_time <= 0)
	{
		left_time = 0;
		state = TIME_OUT;
	}

	string info;
	switch (state)
	{
	case FALLDOWN:
		info = "飞机已坠毁。";
		break;
	case CONFLICT_WITH_OTHER:
		info = "发生撞机。";
		break;
	case TIME_OUT:
		info = "回合结束。";
		break;
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static bool gameover_has_popup = false;
	if (!info.empty())
	{
		pause = true;
		if (gameover_has_popup == false)
		{
			controller->effectPlayer.Play(EFFECT_BUTTON);
			gameover_has_popup = true;
		}

		ImGui::OpenPopup(u8"游戏结束");
	}

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(u8"游戏结束", NULL, ImGuiWindowFlags_AlwaysAutoResize))//ImGuiWindowFlags_NoCollapse
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
			ImGui::Text(string2utf8("新记录:" + to_string(shot_down)).c_str());
			ImGui::InputText("", buf, 16);
		}

		if (ImGui::Button(u8"确定"))
		{
			controller->effectPlayer.Play(EFFECT_BUTTON);
			if (highscore)
			{
				controller->billboard.RefreshRecord(utf82string(buf), shot_down);
			}

			gameover_has_popup = false;
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
		vec3 cpos = front * watch_dist;//初始化摄像机位置为正前方watch_dist距离处
		mat4 R_cpos(1.0f);
		R_cpos = translate(R_cpos, pos);//摄像机target移动至玩家位置
		R_cpos = rotate(R_cpos, radians(watch_angle), right);//摄像机位置向后旋转

		cpos = vec3(R_cpos * vec4(cpos, 1));//w分量需为1，否则位移不生效

		camera->SetPosition(cpos);
		//camera->SetPosition(pos+vec3(0, 5, 15));//
		camera->SetDirection(front);
		camera->SetUp(up);
	}

	//更新并绘制子弹、敌机
	UpdateAndDrawBullets(dt);
	UpdateAndDrawEnemies(dt);


	//渲染地面
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

		terraShader.Uniform("Tex", texGround);
		terras.Draw(camera->GetPosition(), camera->GetDirection());
	}



	//绘制玩家
	{

		modelShader.UseProgram();
		mat4 model(1.0f);

		//quat q = quatLookAt(front, up);
		//mat4 qm = mat4_cast(q);

		//model = mat4(1.0f);
		//model = translate(model, pos);
		//model *= qm;

		vec3 plane_front = front;
		vec3 plane_up = up;
		vec3 plane_right = right;
		UpdateDirVectorByDelta(plane_front, plane_up, plane_right, dpitch, dyaw);//将机身以当前未回正角度作偏转

		//使机身始终朝向front，以up为上轴
		mat4 Mlookat = lookAt(pos, pos + plane_front, plane_up);
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
		//调试用
		{
			//替代玩家的box
			model = mat4(1.0f);
			model = translate(model, pos);
			modelShader.Uniform("model", model);

			box.Bind();
			box.DrawTriangles();

		}
		{
			//front axis
			mat4 Mlookat = lookAt(pos, pos + front, up);
			model *= glm::inverse(Mlookat);

			model = translate(model, vec3(0, 0, -1.5f));
			model = scale(model, vec3(0.1f, 0.1f, 3.0f));
			modelShader.Uniform("model", model);

			box.Bind();
			box.DrawTriangles();
		}
		{
			////up axis
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
			//right axis
			//mat4 Mlookat = lookAt(pos, pos + right, up);

			//model = mat4(1.0f);
			////model = rotate(model, -glm::pi<float>() / 2.0f, up);
			//model *= glm::inverse(Mlookat);
			////model = translate(model, pos);
			////model = rotate(model, radians(-dyaw), up);

			//model = translate(model, vec3(0, 0, -1.5f));
			//model = scale(model, vec3(0.1f, 0.1f, 3.0f));
			////model = translate(model, vec3(1.5f, 0, 0));
			////model = scale(model, vec3(3.0f, 0.1f, 0.1f));
			//modelShader.Uniform("model", model);

			//box.Bind();
			//box.DrawTriangles();
		}
#endif
	}

	//绘制天空盒
	{
		skybox->Draw(camera->GetViewMatrix(), camera->GetProjection());

		camera->RefreshTime();
	}

	//绘制UI
	glDisable(GL_DEPTH_TEST);

	//绘制速度、高度 背景白块
	pureRenderer.DrawCenter(vec4(1, 1, 1, 0.5), vec2(W / 2.0f - 400.0f, H / 2.0f), vec2(180, 40));
	pureRenderer.DrawCenter(vec4(1, 1, 1, 0.5), vec2(W / 2.0f + 400.0f, H / 2.0f), vec2(180, 40));

	//绘制速度、高度 旁的三角
	renderer.DrawCenter(texArrow, vec2(W / 2.0f - 400.0f + 200.0f, H / 2.0f), vec2(40, 40));
	renderer.DrawCenter(texArrow, vec2(W / 2.0f + 400.0f - 200.0f, H / 2.0f), vec2(-40, 40));

	//绘制摄像机、飞机准心
	renderer.DrawCenter(texSpotCamera, vec2(W / 2.0f + dyaw * 40.0f, H / 2.0f + dpitch * 40.0f), vec2(30, 20));//以dyaw和dpitch偏移准心位置
	renderer.DrawCenter(texSpotPlane, vec2(W / 2.0f, H / 2.0f), vec2(30, 20));

	//绘制文字
	freetype.DrawByPixel(renderer, to_string(int(v)) + " km/h", W / 2.0f - 400.0f - 80.0f, H / 2.0f - 15.0f);
	freetype.DrawByPixel(renderer, to_string(int(pos.y)) + " m", W / 2.0f + 400.0f - 80.0f, H / 2.0f - 15.0f);
	freetype.DrawByPixel(renderer, string("score: ") + to_string(shot_down), 80.0f, H - 120.0f);
	freetype.DrawByPixel(renderer, string("left time: ") + to_string((int)left_time), W - 400.0f, H - 120.0f);

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

	//绘制IMGUI
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//更新标题栏
	stringstream ss;
	//ss << "pos=" << pos.x << " " << pos.y << " " << pos.z;
	//ss << " front=" << front.x << " " << front.y << " " << front.z;
	ss << " dpitch=" << dpitch << " dyaw=" << dyaw;
	ss << " real_pitch=" << real_pitch << " real_yaw=" << real_yaw;
	ss << " bullets num=" << bullets.size();
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
	if (pause)
		return;

	static float lastX, lastY;
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

	float x_sensitivity = 0.005f;
	float y_sensitivity = 0.01f;

	dyaw += xoffset * x_sensitivity;
	dpitch += yoffset * y_sensitivity;

	const float pitch_limit = 45.0f;
	if (dpitch > pitch_limit)
		dpitch = pitch_limit;
	if (dpitch < -pitch_limit)
		dpitch = -pitch_limit;

	const float yaw_limit = 30.0f;
	if (dyaw > yaw_limit)
		dyaw = yaw_limit;
	if (dyaw < -yaw_limit)
		dyaw = -yaw_limit;

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
		//新建子弹，速度增加800
		Bullet* bullet = new Bullet(pos, front * v + (front * 800.0f));
		bullets.push_back(bullet);

		controller->effectPlayer.Play(EFFECT_GUN);
	}

	for (auto it = bullets.begin(); it != bullets.end();)
	{
		auto& bullet = *it;
		if (!pause)
			bullet->UpdatePos(dt);
		bool dead = bullet->pos.y <= 0;//子弹已坠地

		if (dead)
		{
			delete bullet;
			it = bullets.erase(it);
		}
		else
		{
			bullet->Draw(modelBullet, modelShader, *camera);
			it++;
		}
	}
}

void SceneLevel::UpdateAndDrawEnemies(float dt)
{
	//敌机低于上限则添加
	if (enemies.size() < enemy_limit)
	{
		uniform_real_distribution<float> uni_angle(-45, 45);//角度范围
		uniform_real_distribution<float> uni_r(200, 8000);//距离范围
		uniform_real_distribution<float> uni_altitude(-100, 100);//高度范围
		for (int i = enemies.size(); i < enemy_limit; ++i)
		{
			vec3 enemy_pos = front * uni_r(eng);//初始化敌机位于原点front方向正前方 距离r处

			enemy_pos.y += uni_altitude(eng);//设置敌机海拔
			if (enemy_pos.y < 200.0f)
				enemy_pos.y += 200.0f;

			mat4 R(1.0f);
			R = translate(R, pos);//平移pos距离
			R = rotate(R, uni_angle(eng), init_up);//以原点进行旋转
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

		float dist = distance(enemy->pos, pos);//和玩家距离
		bool dead = dist > 1000;//超出限定距离销毁敌机

		if (!dead)
		{
			//遍历子弹进行碰撞检测
			for (auto bullet : bullets)
			{
				float dist = distance(bullet->pos, enemy->pos);
				if (Conflict(*bullet, *enemy))
					//if (dist < 20)
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
			if (state == NORMAL && dist < 10.0f)//和玩家撞机
			{
				controller->effectPlayer.Play(EFFECT_BOMB);
				state = CONFLICT_WITH_OTHER;
			}

			enemy->Draw(plane, modelShader, *camera);
			it++;

		}
	}
}

void SceneLevel::UpdatePlayerPos(float dt)
{
	real_pitch += dpitch;
	real_yaw += dyaw;

	//CalcDirVectorByRotateMatrix(dt);
	//CalcDirVectorByQuat(dt);
	UpdateDirVectorByDelta(front, up, right, dpitch,dyaw);

	if (dpitch > 0)
		dpitch -= dt * 1.0f;
	if (dpitch < 0)
		dpitch += dt * 1.0f;
	if (abs(dpitch) < dt * 1.0f)
		dpitch = 0;

	if (dyaw > 0)
		dyaw -= dt * 1.0f;
	if (dyaw < 0)
		dyaw += dt * 1.0f;
	if (abs(dyaw) < dt * 1.0f)
		dyaw = 0;

	//
	static float volume = 30;
	if (controller->keys[GLFW_KEY_W] && v < 2200.0f)//按下W且低于上限
	{
		if (volume < 100.0f)//音量低于100则逐步升高
			volume += dt * 50.0f;
		else
			volume = 100.0f;
		controller->jetPlayer.SetVolume(volume);
		v += dt * 80.0f;//加速
	}
	else
	{
		//没按W则逐步降低音量
		if (volume > 30.0f)
			volume -= dt * 80.0f;
		else
			volume = 30.0f;
		controller->jetPlayer.SetVolume(volume);
	}

	if (controller->keys[GLFW_KEY_S] && v > 180.0f)//按下S且速度高于下限
	{
		v -= dt * 80.0f;//减速
	}

	//缓慢自然减速
	v -= dt * 1.0f;

	//按照方向和速度更新位置
	float v_m_per_sec = v / 3.6f;
	pos += front * (dt * v_m_per_sec);

	//刷新倒计时
	left_time -= dt;

	if (state == NORMAL)
	{
		if (pos.y <= terras.GetAltitude(pos))
		{
			controller->effectPlayer.Play(EFFECT_BOMB);
			state = FALLDOWN;
		}
	}
}

void SceneLevel::CalcDirVectorByRotateMatrix(float dt)
{
	{
		mat4 R_front(1.0f);
		R_front = rotate(R_front, glm::radians(real_pitch), init_right);
		front = vec3(R_front * vec4(init_front, 0));
		up = vec3(R_front * vec4(init_up, 0));
		right = vec3(R_front * vec4(init_right, 0));
	}

	//cout << "front up=" << glm::degrees(acos(dot(front, up) / (length(front) * length(up)))) << endl;

	{
		mat4 R(1.0f);
		R = rotate(R, radians(-real_yaw), up);

		front = vec3(R * vec4(front, 0));
		right = vec3(R * vec4(right, 0));
	}

	////cout << "right front=" << glm::degrees(acos(dot(right,front)/(length(right)*length(front))))<< endl;

	{
		mat4 R(1.0f);
		R = rotate(R, radians(-real_yaw), front);

		up = vec3(R * vec4(up, 0));
		right = vec3(R * vec4(right, 0));
	}

	//cout << "up right=" << glm::degrees(acos(dot(up, right)/(length(up)*length(right))))<< endl;

	//cout << "front: " << front << endl;
}

void SceneLevel::CalcDirVectorByQuat(float dt)
{
	{

		float rad = radians(real_pitch / 2.0f);
		quat q(cos(rad), sin(rad) * init_right);
		//quat q = quat_cast(mat3(R_front));

		front = q * init_front * conjugate(q);
		up = q * init_up * conjugate(q);
		right = q * init_right * conjugate(q);
	}

	//cout << "front up=" << glm::degrees(acos(dot(front, up) / (length(front) * length(up)))) << endl;

	{
		float rad = radians(-real_yaw) / 2.0f;
		quat q(cos(rad), sin(rad) * up);
		front = q * front * conjugate(q);
		right = q * right * conjugate(q);
	}

	//cout << "right front=" << glm::degrees(acos(dot(right,front)/(length(right)*length(front))))<< endl;

	{
		float rad = radians(-real_yaw) / 2.0f;
		quat q(cos(rad), sin(rad) * front);
		up = q * up * conjugate(q);
		right = q * right * conjugate(q);
	}

	//cout << "up right=" << glm::degrees(acos(dot(up, right)/(length(up)*length(right))))<< endl;
}

void SceneLevel::UpdateDirVectorByDelta(glm::vec3& front, glm::vec3& up, glm::vec3& right,float dpitch,float dyaw)
{
	{
		//以right为轴，旋转3个轴
		float rad = radians(dpitch / 2.0f);
		quat q(cos(rad), sin(rad) * right);
		//quat q = quat_cast(mat3(R_front));

		front = q * front * conjugate(q);
		up = q * up * conjugate(q);
		right = q * right * conjugate(q);
	}

	//cout << "front up=" << glm::degrees(acos(dot(front, up) / (length(front) * length(up)))) << endl;

	{
		//以up为轴，旋转front,right
		float rad = radians(-dyaw) / 2.0f;
		quat q(cos(rad), sin(rad) * up);
		front = q * front * conjugate(q);
		right = q * right * conjugate(q);
	}

	//cout << "right front=" << glm::degrees(acos(dot(right,front)/(length(right)*length(front))))<< endl;

	{
		//以front为轴，旋转up,right
		float rad = radians(-dyaw) / 2.0f;
		quat q(cos(rad), sin(rad) * front);
		up = q * up * conjugate(q);
		right = q * right * conjugate(q);
	}
}
