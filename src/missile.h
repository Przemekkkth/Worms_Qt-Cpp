#ifndef MISSILE_H
#define MISSILE_H
#include "physics_object.h"
#include <vector>
#include <utility>

class Missile : public PhysicsObject
{
public:
    Missile(float x = 0.0f, float y = 0.0f, float _vx = 0.0f, float _vy = 0.0f);
    virtual void Draw(GameScene *scene, float fOffsetX, float fOffsetY) override;
    virtual int BounceDeathAction() override;
private:
    static std::vector<std::pair<float, float>> vecModel;
};

#endif // MISSILE_H
