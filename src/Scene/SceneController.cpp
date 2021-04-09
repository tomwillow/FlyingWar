#include "SceneController.h"

#include "ResMacro.h"
#include "ResValue.h"

#include "SceneCover.h"
#include "SceneLevel.h"

#include <GLFW/glfw3.h>

using namespace std;

SceneController::SceneController(GLFWwindow* in_glfw_window) :
	glfw_window(in_glfw_window),
	bgmPlayer(SOUND_BGM),
	jetPlayer(SOUND_JET),
	billboard(RECORD_FILENAME)
{
	effectPlayer.AddFile(EFFECT_GUN, SOUND_GUN);
	effectPlayer.AddFile(EFFECT_BOMB, SOUND_BOMB);
	effectPlayer.AddFile(EFFECT_BUTTON, SOUND_BUTTON);
	effectPlayer.PrepareDevice(10);

	GoCover();
	//GoLevel(0);
}

void SceneController::GoCover()
{
	bgmPlayer.Stop();
	jetPlayer.Stop();
	scene = make_unique<SceneCover>(this);
}

void SceneController::GoLevel(int i)
{
	switch (i)
	{
	case 0:
		bgmPlayer.Play();
		bgmPlayer.SetVolume(50);
		jetPlayer.Play();
		jetPlayer.SetVolume(30);
		scene = make_unique<SceneLevel>(this);
		break;
	default:
		assert("wrong level number");
	}
}

void SceneController::BeforeGLClear(float dt)
{
	scene->BeforeGLClear(dt);
}

void SceneController::Render(float dt)
{
	scene->Render(dt);
}

void SceneController::AfterSwapBuffers(float dt)
{
	scene->AfterSwapBuffers(dt);
}

void SceneController::OnKey(int key, int action)
{
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = GL_TRUE;
		else if (action == GLFW_RELEASE)
			keys[key] = GL_FALSE;
	}

	scene->OnKey(key, action);
}

void SceneController::OnSize(int in_width, int in_height)
{
	scene->OnSize(in_width, in_height);
}

void SceneController::OnMouseMove(double xpos, double ypos)
{
	scene->OnMouseMove(xpos, ypos);
}

void SceneController::OnMouseButton(int button, int action)
{
	scene->OnMouseButton(button, action);
}