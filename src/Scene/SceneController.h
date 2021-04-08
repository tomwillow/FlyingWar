#pragma once

#include "Scene.h"

#include "MusicPlayer.h"
#include "WaveOutEffect.h"
#include "Billboard.h"

#ifdef min
#undef min
#endif

#include <memory>

struct GLFWwindow;
class SceneController
{
public:
	MusicPlayer bgmPlayer;
	WaveOutEffect effectPlayer;
	MusicPlayer jetPlayer;
	Billboard billboard;

	GLFWwindow* glfw_window;
	bool keys[1024];

	SceneController(GLFWwindow *in_glfw_window);

	void GoCover();
	void GoLevel(int i);

	void BeforeGLClear(float dt);
	void Render(float dt);
	void AfterSwapBuffers(float dt);

	void OnKey(int key, int action);
	void OnSize(int in_width, int in_height);
	void OnMouseMove(double xpos, double ypos);
	void OnMouseButton(int button, int action);

private:
	std::unique_ptr<Scene> scene;
};

