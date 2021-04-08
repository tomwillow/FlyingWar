#include "TObject.h"

bool Conflict(const TObject& a, const TObject& b)
{
    float dist = glm::distance(a.pos, b.pos);
    return dist<(a.r+b.r);
}

TObject::TObject(glm::vec3 in_pos, glm::vec3 in_v, float in_r):
    pos(in_pos),v(in_v),r(in_r)
{
}

void TObject::UpdatePos(float dt)
{
    v += dt * G;
    pos += dt * v;
}
