#pragma once
#include "Scene.h"

#include "PureRenderer.h"

#include "TCamera.h"
#include "TSkyBox.h"
#include "Model.h"
#include "TerraManager.h"
#include "TFreeType.h"
#include "TBox.h"

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
	virtual void AfterLose(std::string info);

private:
	const glm::vec3 born_pos = { 0,500,0 };
	const glm::vec3 init_dir = { 0,0,-1 };
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 up = { 0,1,0 };
	glm::vec3 right = { 1,0,0 };
	float yaw=0,pitch = 0;
	float v = 500.0f;//单位km/h
	float dist = 20;
	float angle = 75.0f;

	TShader pureShader;//纯色着色器
	PureRenderer pureRenderer;//纯色渲染器
	TSpiritVertex spirit;
	TFreeTypeDictionary freetype;
	TTexture texArrow, texSpotPlane, texSpotCamera;
	TBox box;

	std::unique_ptr<TCamera> camera;
	std::unique_ptr<TSkyBox> skybox;

	TShader terraShader;
	TerraManager terras;

	Model plane;
	TShader modelShader;
};

inline std::ostream& operator<<(std::ostream& out, glm::vec2 v)
{
	out << v.x << " " << v.y;
	return out;
}