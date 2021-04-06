#pragma once

#include <string>
#include <memory>

#include "TShader.h"
#include "TTexture.h"
#include "SpiritRenderer.h"

class SceneController;
class Scene
{
public:
	Scene(SceneController * in_controller);
	virtual ~Scene() = default;

	virtual void Render(float dt) = 0;
	virtual void AfterSwapBuffers(float dt) {}

	virtual void OnKey(int key, int action) = 0;
	virtual void OnSize(int in_width, int in_height);
	virtual void OnMouseMove(double xpos, double ypos) {}
	virtual void OnMouseButton(int button, int action) {}
protected:
	float W, H;
	SceneController *controller;
	TShader spiritShader;
	SpiritRenderer renderer;
};

