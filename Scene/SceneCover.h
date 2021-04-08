#pragma once
#include "Scene.h"

class SceneCover :
    public Scene
{
public:
	SceneCover(SceneController* in_controller);

	void DrawBackground();
	void DrawBackground(SpiritRenderer& in_renderer);
	void DrawBackground(SpiritRenderer& in_renderer, glm::vec2 pos, glm::vec2 size);

	virtual void BeforeGLClear(float dt) override;
	virtual void Render(float dt) override;
	virtual void OnKey(int key, int action) override;
private:
	TTexture texBackground;
};

