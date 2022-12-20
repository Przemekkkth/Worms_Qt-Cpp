#ifndef DEBRIS_H
#define DEBRIS_H
#include "physics_object.h"
#include <vector>
#include <utility>

class Debris : public PhysicsObject
{
public:
    Debris(float x = 0.0f, float y = 0.0f);
    virtual ~Debris();
    virtual void Draw(GameScene *scene, float fOffsetX, float fOffsetY) override;
    virtual int BounceDeathAction() override;
private:
    static std::vector<std::pair<float, float>> vecModel;
};

#endif // DEBRIS_H
