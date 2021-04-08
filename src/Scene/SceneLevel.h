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
	const glm::vec3 born_pos = { 0,500,0 };
	const glm::vec3 init_dir = { 0,0,-1 };
	const glm::vec3 init_up = { 0,1,0 };
	const glm::vec3 init_right = { 1,0,0 };
	bool pause = false;
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 up = init_up;
	glm::vec3 right = init_right;
	float yaw=0,pitch = 0;
	float v = 500.0f;//单位km/h
	float dist = 20;
	float angle = 75.0f;
	bool player_is_dead = false;
	bool falldown = false;
	float left_time = 90.0f;
	int shot_down = 0;

	TShader pureShader;//纯色着色器
	PureRenderer pureRenderer;//纯色渲染器
	TSpiritVertex spirit;
	TFreeTypeDictionary freetype;
	TTexture texArrow, texSpotPlane, texSpotCamera,texSpot;
	TBox box;

	std::unique_ptr<TCamera> camera;
	std::unique_ptr<TSkyBox> skybox;

	TShader terraShader;
	TerraManager terras;

	Model plane;
	TShader modelShader;

	Model modelBullet;
	std::vector<Bullet*> bullets;
	std::vector<Enemy*> enemies;

	void UpdateAndDrawBullets(float dt);
	void UpdateAndDrawEnemies(float dt);
	void UpdatePlayerPos(float dt);
};

inline std::ostream& operator<<(std::ostream& out, glm::vec2 v)
{
	out << v.x << " " << v.y;
	return out;
}