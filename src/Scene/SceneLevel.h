#pragma once
#include "Scene.h"

#include "PureRenderer.h"

#include "TCamera.h"
#include "TSkyBox.h"
#include "Model.h"
#include "TerraManager.h"
#include "TFreeType.h"
#include "TBox.h"
#include "Bullet.h"
#include "Enemy.h"

#include <vector>
#include <fstream>
#include <random>

class SceneLevel :
    public Scene
{
public:
	SceneLevel(SceneController* in_controller);
	virtual ~SceneLevel();

	virtual void BeforeGLClear(float dt) override;
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
	virtual void AfterLose(std::string info);

private:
	enum PlayerState{NORMAL,FALLDOWN,CONFLICT_WITH_OTHER,TIME_OUT};
	PlayerState state = NORMAL;

	bool pause = false;//是否暂停游戏
	const glm::vec3 born_pos = { 0,500,0 };
	const glm::vec3 init_front = { 0,0,-1 };
	const glm::vec3 init_up = { 0,1,0 };
	const glm::vec3 init_right = { 1,0,0 };
	glm::vec3 pos=born_pos;
	glm::vec3 front=init_front;
	glm::vec3 up = init_up;
	glm::vec3 right = init_right;
	float dyaw=0,dpitch = 0;
	float real_pitch = 0;
	float real_yaw = 0;

	float v = 500.0f;//初始速度，单位km/h
	float watch_dist = 20;//摄像机离玩家距离
	float watch_angle = 165.0f;//摄像机观察角度，从-z轴（正前方）向后旋转
	//bool player_is_dead = false;
	//bool falldown = false;
	float left_time = 90.0f;//一回合时长，单位秒
	int shot_down = 0;//击落敌机数量

	//UI
	TShader pureShader;//纯色着色器
	PureRenderer pureRenderer;//纯色渲染器
	TSpiritVertex spirit;
	TFreeTypeDictionary freetype;
	TTexture texArrow, texSpotPlane, texSpotCamera,texSpot;
#ifdef _DEBUG
	TBox box;
#endif

	//天空盒
	std::unique_ptr<TCamera> camera;
	std::unique_ptr<TSkyBox> skybox;

	//地面
	TShader terraShader;
	TerraManager terras;
	TTexture texGround;

	//飞机
	Model plane;
	TShader modelShader;

	//子弹
	Model modelBullet;
	std::vector<Bullet*> bullets;

	//敌人
	std::vector<Enemy*> enemies;
	const int enemy_limit = 20;//敌机总数上限

	void UpdateAndDrawBullets(float dt);//pause的影响已在函数内生效
	void UpdateAndDrawEnemies(float dt);//pause的影响已在函数内生效
	void UpdatePlayerPos(float dt);//函数内未受pause影响，需在外部判断
	void CalcDirVectorByRotateMatrix(float dt);
	void CalcDirVectorByQuat(float dt);
	void UpdateDirVectorByDelta(glm::vec3 &front, glm::vec3 &up, glm::vec3 &right, float dpitch, float dyaw);
};

inline std::ostream& operator<<(std::ostream& out, glm::vec2 v)
{
	out << v.x << " " << v.y;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, glm::vec3 v)
{
	out << v.x << " " << v.y<< " "<<v.z;
	return out;
}