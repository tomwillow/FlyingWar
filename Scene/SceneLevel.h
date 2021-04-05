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
	const float height_km = 0.1;//一屏等价于0.1km
	const float step_km = 0.05;//轨道y方向间距
	const float road_width = 0.05;//道路宽50米
	const glm::vec2 car_size = glm::vec2(0.008f, 0.008f);//汽车尺寸，长宽8米
	const float turnning_rate = 0.4f;//弯道出现概率 范围0-1
	const float start_line_outter_than_start_pos = 0.01f;//y方向比赛车突出10米
	const float track_km = 300.0 * (0.5 / 60.0);//v*t=s (km)  以200km/h时速，全程0.5分钟得到的路程
	const glm::vec2 road_line=glm::vec2(0.002f, 0.02f);//道路中心线尺寸
	const glm::vec2 curbstone = glm::vec2(0.001f, 0.003f);//单块路沿石尺寸

	const glm::vec2 max_v=glm::vec2(20, 300);//x y 方向最大限速 单位=km/h
	const glm::vec2 min_v=glm::vec2(-20, -15);//x y 方向最低限速 单位=km/h

	float width_km;//一屏等价宽度

	glm::vec2 start_pos;//玩家起始坐标
	glm::vec2 start_line_center;//起点线中心坐标
	glm::vec2 end_line_center;//终点线中心坐标


	glm::vec2 KmToPx(glm::vec2 km);//km转换为pixel
	glm::vec2 PxToKm(glm::vec2 px);//pixel转换为km

	struct Line
	{
		glm::vec2 start;
		glm::vec2 end;
	};
	bool IsCollision(const Player& car1, const Player& car2);
	void TwoCarCollision(Player& car1, Player& car2);
	float GetDistance(Line line, glm::vec2 pt);//返回点到直线的距离，位于直线左边值为-，位于直线右边值为+
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

	Player player;//玩家
	std::vector<Player> rivals;//竞争对手

	TShader pureShader;//纯色着色器
	PureRenderer pureRenderer;//纯色渲染器

	void Update(float dt);//接收玩家操作，更新所有物体状态
	void UpdateViewMatrix();//更新视图矩阵

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