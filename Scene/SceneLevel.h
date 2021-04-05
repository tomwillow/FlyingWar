#pragma once
#include "Scene.h"

#include "Player.h"
#include "PureRenderer.h"

#include <vector>
#include <fstream>
#include <random>

class SceneLevel :
    public Scene
{
public:
	SceneLevel(SceneController* in_controller);

	virtual void Render(float dt) override;
	virtual void AfterSwapBuffers(float dt) override;
	virtual void OnKey(int key, int action) override;
	virtual void OnSize(int in_width, int in_height) override;
	virtual void OnMouseMove(double xpos, double ypos) override;
	virtual void OnMouseButton(int button, int action) override;
protected:
	//
	std::default_random_engine eng;

	virtual void AfterWin();
	virtual void AfterLose(std::string winner_name);

private:
	const float height_km = 0.1;//һ���ȼ���0.1km
	const float step_km = 0.05;//���y������
	const float road_width = 0.05;//��·��50��
	const glm::vec2 car_size = glm::vec2(0.008f, 0.008f);//�����ߴ磬����8��
	const float turnning_rate = 0.4f;//������ָ��� ��Χ0-1
	const float start_line_outter_than_start_pos = 0.01f;//y���������ͻ��10��
	const float track_km = 300.0 * (0.5 / 60.0);//v*t=s (km)  ��200km/hʱ�٣�ȫ��0.5���ӵõ���·��
	const glm::vec2 road_line=glm::vec2(0.002f, 0.02f);//��·�����߳ߴ�
	const glm::vec2 curbstone = glm::vec2(0.001f, 0.003f);//����·��ʯ�ߴ�

	const glm::vec2 max_v=glm::vec2(20, 300);//x y ����������� ��λ=km/h
	const glm::vec2 min_v=glm::vec2(-20, -15);//x y ����������� ��λ=km/h

	float width_km;//һ���ȼۿ��

	glm::vec2 start_pos;//�����ʼ����
	glm::vec2 start_line_center;//�������������
	glm::vec2 end_line_center;//�յ�����������


	glm::vec2 KmToPx(glm::vec2 km);//kmת��Ϊpixel
	glm::vec2 PxToKm(glm::vec2 px);//pixelת��Ϊkm

	struct Line
	{
		glm::vec2 start;
		glm::vec2 end;
	};
	bool IsCollision(const Player& car1, const Player& car2);
	void TwoCarCollision(Player& car1, Player& car2);
	float GetDistance(Line line, glm::vec2 pt);//���ص㵽ֱ�ߵľ��룬λ��ֱ�����ֵΪ-��λ��ֱ���ұ�ֵΪ+
	Line GetCenterLine(Player& one);
	Line GetLeftLimitLine(Player& one);
	Line GetRightLimitLine(Player& one);
	void CollisionWithLine(Player& one);
	bool CollisionWithLeftLine(Line line, Player& one);
	bool CollisionWithRightLine(Line line, Player& one);

	void DrawDashLine(glm::vec2 start, glm::vec2 end);
	void DrawCurbstone(glm::vec2 start, glm::vec2 end);

	std::vector<glm::vec2> track;
	void InitTrack();
	void DrawTrack();

	int GetTrackIndex(glm::vec2 pos);

	Player player;//���
	std::vector<Player> rivals;//��������

	TShader pureShader;//��ɫ��ɫ��
	PureRenderer pureRenderer;//��ɫ��Ⱦ��

	void Update(float dt);//������Ҳ�����������������״̬
	void UpdateViewMatrix();//������ͼ����

	void UpdatePlayerPos(float dt);

	std::unique_ptr<TTexture> tex_curbstone;

	std::unique_ptr<TVertexArray> vertex_road;
	void CreateVertexRoad();
	void DrawVertexRoad();
};

inline std::ostream& operator<<(std::ostream& out, glm::vec2 v)
{
	out << v.x << " " << v.y;
	return out;
}

bool IsOverlap(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);