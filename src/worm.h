#ifndef WORM_H
#define WORM_H
#include "physics_object.h"
#include <QPixmap>

class Worm : public PhysicsObject
{
public:
    Worm(float x, float y);
    virtual void Draw(GameScene *scene, float fOffsetX, float fOffsetY) override;
    virtual int BounceDeathAction() override;

private:
    QPixmap m_pixmap;
};

#endif // WORM_H
