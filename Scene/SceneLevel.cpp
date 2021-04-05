
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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <sstream>

#include <Windows.h>

#pragma comment(lib,"opengl32.lib")

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;
using namespace glm;

SceneLevel::SceneLevel(SceneController* in_controller) :
	Scene(in_controller),
	pureShader(SHADER_PURE_VERTEX, SHADER_PURE_FRAGMENT),
	pureRenderer(pureShader),
	player(TEXTURE_CAR),
	tex_curbstone(make_unique<TTexture>(TEXTURE_CURBSTONE))
{
	eng.seed(GetTickCount());

	//
	rivals.push_back(Player(TEXTURE_CAR_BLUE));
	rivals.push_back(Player(TEXTURE_CAR_YELLOW));
	rivals.push_back(Player(TEXTURE_CAR_GREEN));

	pureShader.UseProgram();
	mat4 view(1.0f);
	pureShader.Uniform("view", view);

	OnSize(W, H);
	glClearColor(0, 0.8, 0.1, 1);

	//
	InitTrack();
}

void SceneLevel::Render(float dt)
{
	//������������״̬��������Ҳ���
	Update(dt);

	float ran_km = PxToKm(player.pos).y;
	float total_km = end_line_center.y - start_line_center.y;
	string title = "SimpleRacer Progress:" + to_string(ran_km) + " / " + to_string(total_km);
	glfwSetWindowTitle(controller->glfw_window, title.c_str());

	//view
	UpdateViewMatrix();

	DrawTrack();

	//������յ�
	vec2 start_line_size(width_km * 0.4, road_line.x);
	pureRenderer.DrawCenter(vec4(1, 0, 0, 1), KmToPx(start_line_center), KmToPx(start_line_size));

	pureRenderer.DrawCenter(vec4(1, 0, 0, 1), KmToPx(end_line_center), KmToPx(start_line_size));

	//��������
	player.Draw(renderer);

	for (auto& car : rivals)
		car.Draw(renderer);
}

void SceneLevel::AfterSwapBuffers(float dt)
{
	//judge
	for (int i=0;i<rivals.size();++i)
	{
		auto& car = rivals[i];
		if (car.pos.y - car.size.y / 2.0f > KmToPx(end_line_center).y)
		{
			AfterLose("rival "+to_string(i+1));
			return;
		}
	}


	if (player.pos.y - player.size.y / 2.0f > KmToPx(end_line_center).y)
	{
		AfterWin();
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

	pureShader.UseProgram();
	mat4 projection(1.0f);
	projection = ortho(0.0f, (float)W, 0.0f, (float)H);

	pureShader.Uniform("projection", projection);


	player.UpdateWindowSize({ in_width,in_height });

	width_km = W * height_km / H;
}

void SceneLevel::OnMouseMove(double xpos, double ypos)
{
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

void SceneLevel::AfterLose(std::string winner_name)
{
	HWND hWnd = glfwGetWin32Window(controller->glfw_window);
	string text = "You lose. The winner is " + winner_name+".";
	if (MessageBox(hWnd, text.c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION) == IDOK)
	{
		controller->GoCover();
		return;
	}
}

glm::vec2 SceneLevel::KmToPx(glm::vec2 km)
{
	return vec2(km.x / width_km * W, km.y / height_km * H);
}

glm::vec2 SceneLevel::PxToKm(glm::vec2 px)
{
	return vec2(px.x * width_km / W, px.y * height_km / H);
}

bool SceneLevel::IsCollision(const Player& car1, const Player& car2)
{
	//float d_limit = car1.size.x / 2.0f;
	//float d = glm::distance(car1.pos, car2.pos);
	//return d < d_limit;
	vec2 p1 = car1.pos - car1.size / 2.0f;
	vec2 p2 = car1.pos + car1.size / 2.0f;
	vec2 p3 = car2.pos - car2.size / 2.0f;
	vec2 p4 = car2.pos + car2.size / 2.0f;
	return IsOverlap(p1, p2, p3, p4);
}

void SceneLevel::TwoCarCollision(Player& car1, Player& car2)
{
	if (IsCollision(car1, car2))
	{
		swap(car1.v, car2.v);
	}
}

float SceneLevel::GetDistance(Line line, glm::vec2 pt)
{
	//ֱ������ʽ ת��Ϊ һ��ʽ
	float A = line.end.y - line.start.y;
	float B = line.start.x - line.end.x;
	float C = -line.start.x * line.end.y + line.end.x * line.start.y;

	float d = (A * pt.x + B * pt.y + C) / sqrt(A * A + B * B);
	return d;
}

SceneLevel::Line SceneLevel::GetCenterLine(Player& one)
{
	int start_index = GetTrackIndex(PxToKm(one.pos));//��ǰ�����ĸ�����
	vec2 start(track[start_index]);
	vec2 end(track[start_index + 1]);
	return { start,end };
}

SceneLevel::Line SceneLevel::GetLeftLimitLine(Player& one)
{
	int start_index = GetTrackIndex(PxToKm(one.pos));//��ǰ�����ĸ�����
	vec2 left_start(track[start_index].x - road_width / 2.0f, track[start_index].y);
	vec2 left_end(track[start_index + 1].x - road_width / 2.0f, track[start_index + 1].y);
	return { left_start,left_end };
}

SceneLevel::Line SceneLevel::GetRightLimitLine(Player& one)
{
	int start_index = GetTrackIndex(PxToKm(one.pos));
	vec2 right_start(track[start_index].x + road_width / 2.0f, track[start_index].y);
	vec2 right_end(track[start_index + 1].x + road_width / 2.0f, track[start_index + 1].y);
	return { right_start,right_end };
}

void SceneLevel::CollisionWithLine(Player& one)
{
	Line left_line = GetLeftLimitLine(one);

	//float left_d = GetDistance(left_line, PxToKm(one.pos));
	//cout << "left d = " << left_d << endl;

	if (CollisionWithLeftLine(left_line, one))
	{
		vec2 p = left_line.end - left_line.start;
		vec3 p3(p, 0);
		mat3 R(1.0f);
		R = rotate(R, radians(-90.0f));

		vec3 n = normalize(R * p3);
		vec3 out = reflect(vec3(one.v, 0), n);
		one.v = out * 0.5f;

		one.frozen_left_key = true;
		one.frozen_time = 0.5f;
		return;
	}

	Line right_line = GetRightLimitLine(one);
	//float right_d = GetDistance(right_line, PxToKm(one.pos));
	//cout << "right d = " << right_d << endl;
	if (CollisionWithRightLine(right_line, one))
	{
		vec2 p = right_line.end - right_line.start;
		vec3 p3(p, 0);
		mat3 R(1.0f);
		R = rotate(R, radians(90.0f));

		vec3 n = normalize(R * p3);
		vec3 out = reflect(vec3(one.v, 0), n);
		one.v = out * 0.9f;

		one.frozen_right_key = true;
		one.frozen_time = 0.5f;
		return;
	}
}

bool SceneLevel::CollisionWithLeftLine(Line line, Player& one)
{
	float d = GetDistance(line, PxToKm(one.pos));

	float d_limit = PxToKm(one.size).x / 2.0f;


	if (d < d_limit)
	{
		//cout << "d (km) = " << d << endl;
		//cout << "Is Collision = " << (d < d_limit) << endl;
		return true;
	}
	return false;
}

bool SceneLevel::CollisionWithRightLine(Line line, Player& one)
{
	float d = GetDistance(line, PxToKm(one.pos));

	float d_limit = PxToKm(one.size).x / 2.0f;


	if (-d < d_limit)
	{
		//cout << "d (km) = " << d << endl;
		//cout << "Is Collision = " << (d < d_limit) << endl;
		return true;
	}
	return false;
}

void SceneLevel::DrawDashLine(glm::vec2 start, glm::vec2 end)
{
	float dist = distance(start, end);
	float abs_rad = acos((end.x - start.x) / dist);

	float dx = road_line.y * cos(abs_rad);
	float dy = road_line.y * sin(abs_rad);

	vec2 c = start;
	c.x += dx / 2.0f;
	c.y += dy / 2.0f;
	while (1)
	{
		if (c.y + dy / 2.0f > end.y)//������������
			break;

		//draw
		pureRenderer.DrawCenter(vec4(1), KmToPx(c), KmToPx(road_line), glm::degrees(abs_rad) + 90.0);

		//����һ�����Σ����Լ����2��
		c.x += dx * 2.0f;
		c.y += dy * 2.0f;
	}

}

void SceneLevel::DrawCurbstone(glm::vec2 start, glm::vec2 end)
{
	float dist = distance(start, end);
	float abs_rad = acos((end.x - start.x) / dist);//��ǣ�����x��Ϊ��׼

	int stone_count = dist / curbstone.y;
	vec2 actual_stone(curbstone.x, dist / stone_count);

	float dx = actual_stone.y * cos(abs_rad);
	float dy = actual_stone.y * sin(abs_rad);

	vec2 c = start;
	c.x += dx / 2.0f;
	c.y += dy / 2.0f;
	for (int i = 0; i < stone_count; ++i)
	{
		//draw
		renderer.DrawCenter(*tex_curbstone, KmToPx(c), KmToPx(actual_stone), glm::degrees(abs_rad) + 90.0);

		//���1��
		c.x += dx;
		c.y += dy;
	}
}

void SceneLevel::InitTrack()
{
	int step_count = track_km / step_km;//������������
	track = vector<vec2>(step_count + 1);//�ڵ�����������������1

	//��������ֱ������
	for (int i = 0; i < track.size(); ++i)
	{
		float x = width_km / 2.0;//xλ���м��
		float y = i * step_km;//y��ڵ����
		track[i] = { x,y };
	}

	//��·����������λ
	float left_limit = width_km/2.0f-road_width;
	float right_limit = width_km/2.0f+road_width;

	int turnning_num = (float)(step_count - 4) * turnning_rate;
	uniform_real_distribution<float> uni_rate;//0-1������ֲ�
	int actual_turnning = 0;//ʵ�������������
	for (int i = 3; i < track.size() - 3; ++i)
	{
		if (actual_turnning < turnning_num && uni_rate(eng) < turnning_rate)//�������С�����ƣ��Ҹ��ʴ������
		{
			//�������
			uniform_real_distribution<float> uni(std::max(left_limit,track[i-1].x-road_width), std::min(right_limit,track[i-1].x+road_width));
			track[i].x = uni(eng);
			actual_turnning++;

		}
		else
		{
			//����ֱ��
			track[i].x = track[i - 1].x;
		}
	}

	//���λ��
	start_pos=vec2(width_km / 2.0f, height_km * 0.25f);
	start_line_center=vec2(start_pos.x, start_pos.y + start_line_outter_than_start_pos);

	//�յ�λ��
	end_line_center = vec2(track.back().x, track.back().y - height_km);

	//��ʼ�����λ��
	player.pos = KmToPx(start_pos);
	player.size = KmToPx(car_size);

	//��ʼ����������λ��
	vector<vec2> start_pos_vec;
	start_pos_vec.push_back(start_pos - vec2(car_size.x * 2.0f, 0));//������
	start_pos_vec.push_back(start_pos + vec2(car_size.x * 2.0f, 0));//����ұ�
	start_pos_vec.push_back(start_pos - vec2(car_size.x * 2.0f, car_size.y * 2.0f));//������
	start_pos_vec.push_back(start_pos + vec2(car_size.x * 2.0f, car_size.y * 2.0f));//����Һ�

	assert(start_pos_vec.size() >= rivals.size());

	//��ʼ����������λ��
	for (int i = 0; i < rivals.size(); ++i)
	{
		rivals[i].pos = KmToPx(start_pos_vec[i]);
		rivals[i].size = KmToPx(car_size);
	}

	//
	CreateVertexRoad();
}

void SceneLevel::DrawTrack()
{
	DrawVertexRoad();

	vec2 prev = track[0];
	for (int i = 1; i < track.size(); ++i)
	{
		vec2 c0 = prev;
		vec2 c1 = track[i];

		///////////////////////

		//���Ƶ�·������
		DrawDashLine(c0, c1);

		//
		vec2 left_start(c0.x - road_width / 2.0f, c0.y);
		vec2 left_end(c1.x - road_width / 2.0f, c1.y);
		//
		vec2 right_start(c0.x + road_width / 2.0f, c0.y);
		vec2 right_end(c1.x + road_width / 2.0f, c1.y);

		//����·��ʯ
		DrawCurbstone(left_start, left_end);
		DrawCurbstone(right_start, right_end);

		////////////////////////

		prev = c1;
	}
}

int SceneLevel::GetTrackIndex(glm::vec2 pos)
{
	int start_index = pos.y / step_km;
	return start_index;
}

bool IsOverlap(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
{
	return p1.y <= p4.y && p3.y <= p2.y && p2.x >= p3.x && p4.x >= p1.x;
}

bool InRange(float v, float v0, float v1)
{
	return v >= v0 && v <= v1;
}
void SceneLevel::Update(float dt)
{
	const float dv_second = 300.0f;//1���ڼ��ٵ� 300km/h
	vec2 dv(0, dv_second * dt);//��֡������ km/h/s
	vec2 dv_px = KmToPx(dv) / 3600.0f;//��֡��������������Ϊ��λ pixel/s

	vec2 max_v_px = KmToPx(max_v) / 3600.0f;// pixel/s
	vec2 min_v_px = KmToPx(min_v) / 3600.0f;// pixel/s

	/////////// ���㾺������
	for (int i = 0; i < rivals.size(); ++i)
	{
		auto& car = rivals[i];
		//Line center_line = GetCenterLine(car);
		Line left_line = GetLeftLimitLine(car);
		Line right_line = GetRightLimitLine(car);
		float d_limit = PxToKm(car.size).x / 2.0f;
		//float center_d= GetDistance(center_line, PxToKm(car.pos));
		float left_d = GetDistance(left_line, PxToKm(car.pos));
		float right_d = GetDistance(right_line, PxToKm(car.pos));

		bool action = false;
		if (left_d < d_limit*2.0f)//��߼�����ײ //-center_d>d_limit || 
		{
			car.TurnRight(max_v_px);
			action = true;
		}
		if (-right_d < d_limit*2.0f)//�ұ߼�����ײ  //center_d>d_limit || 
		{
			car.TurnLeft(min_v_px);
			action = true;
		}

		if (action==false && car.v.y < max_v_px.y * 1.0f)//����
			car.v.y += dv_px.y;

		car.pos += car.v * dt;

		//��·��ʯ������ײ���
		CollisionWithLine(car);

		//���ú������������Ѿ�������������

		//����֮�以��
		for (int j = 0; j < rivals.size(); ++j)
		{
			if (j == i) continue;
			TwoCarCollision(car, rivals[j]);
		}
	}
	////////// �������ּ������

	//���������ͣ��ٶ��½�
	if (player.v.y > 0)
		player.v.y -= dv_px.y * 0.1f;

	//�����Ϸ����
	if (controller->keys[GLFW_KEY_UP] && player.v.y < max_v_px.y)
	{
		player.v.y += dv_px.y;
	}

	//�����·����
	if (controller->keys[GLFW_KEY_DOWN] && player.v.y > min_v_px.y)
	{
		player.v.y -= dv_px.y;
	}

	//���������
	if (!player.frozen_left_key)//δ���ڶ���״̬
	{
		if (controller->keys[GLFW_KEY_LEFT])
		{
			player.TurnLeft(min_v_px);
		}
	}
	else
	{
		//cout << "left is locked." << endl;
		player.frozen_time -= dt;//��С����ʱ��
		if (player.frozen_time <= 0)
		{

			player.frozen_left_key = false;//�ⶳ
			//player.v.x = 0;
		}
	}

	//�����ҷ����
	if (!player.frozen_right_key)
	{
		if (controller->keys[GLFW_KEY_RIGHT])
		{
			player.TurnRight(max_v_px);
		}
	}
	else
	{
		//cout << "right is locked." << endl;
		player.frozen_time -= dt;
		if (player.frozen_time <= 0)
		{

			player.frozen_right_key = false;
			//player.v.x = 0;
		}
	}


	//cout << "player.v(km) = " << PxToKm(player.v) * 3600.0f << endl;

	player.pos += player.v * dt;

	//����ˣ���ûԽ�������ߣ������Ʋ��ܺ���
	if (player.v.y < 0 && player.pos.y < KmToPx(start_line_center).y)
	{
		player.pos.y = KmToPx(start_line_center).y;
	}

	//collision
	{
		CollisionWithLine(player);
		for (auto& car : rivals)
			TwoCarCollision(player, car);
	}
}
void SceneLevel::UpdateViewMatrix()
{
	vec2 ran_dist_px = player.pos - KmToPx(start_pos);

	mat4 view(1.0f);
	view = translate(view, vec3(-ran_dist_px.x, -ran_dist_px.y, 0));
	shader.UseProgram();

	shader.Uniform("view", view);

	pureShader.UseProgram();
	pureShader.Uniform("view", view);
}
void SceneLevel::UpdatePlayerPos(float dt)
{
	float zone_x_max = W * sceneWidthMulti;

	//����λ��
	player.pos += player.v * dt;


}

void SceneLevel::CreateVertexRoad()
{
	int step_count = track_km / step_km;
	int road_data_point_count = step_count * 6;//2 triangles
	unsigned int road_data_float_count = road_data_point_count * 4;//x y s t
	float* road_data = new float[road_data_float_count];

	vec2 prev = track[0];
	for (int i = 1; i < track.size(); ++i)
	{
		vec2 c0 = prev;
		vec2 c1 = track[i];

		///////////////////////

		//
		vec2 left_start(c0.x - road_width / 2.0f, c0.y);
		vec2 left_end(c1.x - road_width / 2.0f, c1.y);
		//
		vec2 right_start(c0.x + road_width / 2.0f, c0.y);
		vec2 right_end(c1.x + road_width / 2.0f, c1.y);

		//
		vec2 p0 = KmToPx(left_start); vec2 t0(0, 0);
		vec2 p1 = KmToPx(right_start); vec2 t1(1, 0);
		vec2 p2 = KmToPx(right_end); vec2 t2(1, 1);
		vec2 p3 = KmToPx(left_end); vec2 t3(0, 1);

		auto FillData = [&](int start_index, vec2 p, vec2 t)
		{
			road_data[start_index + 0] = p.x;
			road_data[start_index + 1] = p.y;
			road_data[start_index + 2] = t.x;
			road_data[start_index + 3] = t.y;
		};

		FillData((i - 1) * 24 + 0, p0, t0);
		FillData((i - 1) * 24 + 4, p1, t1);
		FillData((i - 1) * 24 + 8, p2, t2);
		FillData((i - 1) * 24 + 12, p0, t0);
		FillData((i - 1) * 24 + 16, p2, t2);
		FillData((i - 1) * 24 + 20, p3, t3);

		////////////////////////

		prev = c1;
	}

	//
	vertex_road = make_unique<TVertexArray>(road_data_float_count * sizeof(float), road_data, std::initializer_list<int>{ 2, 2 });

	delete[] road_data;
}

void SceneLevel::DrawVertexRoad()
{
	pureShader.UseProgram();
	mat4 model(1.0f);
	pureShader.Uniform("model", model);
	pureShader.Uniform("color", vec4(0.5,0.5,0.5, 1));

	vertex_road->Bind();
	vertex_road->DrawTriangles();
}
