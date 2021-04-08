#pragma once
#include "TObject.h"
#include "TShader.h"
#include "TCamera.h"
#include "Model.h"

class Enemy :
    public TObject
{
public:
    bool show;
    Enemy(glm::vec3 in_pos, glm::vec3 in_target, float v = 500.0f);

    void Draw(Model &model,TShader& shader, TCamera& camera);

    glm::vec2 GetScreenCoord(glm::vec2 screenSize);

    virtual void UpdatePos(float dt) override;
private:
    const float r = 200.0f;

    glm::vec2 clipCoord;
};

